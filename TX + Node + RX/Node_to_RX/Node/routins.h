#ifndef ROUTINS_H
#define ROUTINS_H

#include "constants.h"  // Include a user-defined constants file (assumed to be provided)

// Function to initialize SPI interfaces
void init_spi() {
  vspi = new SPIClass(VSPI);  // Create an instance of SPIClass for VSPI
  hspi = new SPIClass(HSPI);  // Create an instance of SPIClass for HSPI

  vspi->begin();  // Initialize VSPI
  hspi->begin();  // Initialize HSPI

  // Begin VSPI with specific pins: SCLK, MISO, MOSI, SS
  vspi->begin(VSPI_SCLK, VSPI_MISO, VSPI_MOSI, VSPI_SS);
  // Begin HSPI with specific pins: SCLK, MISO, MOSI, SS
  hspi->begin(HSPI_SCLK, HSPI_MISO, HSPI_MOSI, HSPI_SS);

  // Set the Slave Select (SS) pins to OUTPUT mode
  pinMode(vspi->pinSS(), OUTPUT);
  pinMode(hspi->pinSS(), OUTPUT);
}

// Function to initialize the SD card
void init_sd() {
  // Attempt to initialize the SD card with VSPI and its SS pin
  if (!SD.begin(VSPI_SS, *vspi)) {
    Serial.println("SD Card Mount Failed");  // Print error message if initialization fails
    return;
  } else {
    Serial.println("SD card initialized successfully");  // Print success message if initialization is successful
  }
  uint8_t cardType = SD.cardType();  // Get the type of the SD card
  if (cardType == CARD_NONE) {
    Serial.println("No SD Card attached");  // Print error message if no SD card is attached
    return;
  }
}

// Function to initialize the RF24 radio
void init_radio() {
  if (!radio.begin(hspi)) {
    Serial.println("Could not initialize radio");
    return;
  } else {
    Serial.println("\n\nRadio initialized successfully");
  }
  radio.setChannel(5);             // Set the radio channel
  radio.setDataRate(RF24_1MBPS);   // Set the data rate
  radio.setPALevel(RF24_PA_HIGH);  // Set the power amplifier level
  radio.openWritingPipe(PIPE);     // Open the writing pipe
}

// Function to send the start signal for image transmission
bool send_image_start() {
  uint8_t buf[PACKET_SIZE] = { 0 };           // Initialize a buffer with zeros
  bool sent = radio.write(buf, sizeof(buf));  // Send the buffer over radio
  if (sent) { return true; };
  return false;
}

// Function to send image size and packet credentials
bool send_image_credentials(int size) {
  uint8_t buf[PACKET_SIZE];
  int full_packets = size / PACKET_SIZE;
  int remainder = size % PACKET_SIZE;

  buf[0] = full_packets & 0xFF;         // Lower 8 bits of full packets
  buf[1] = (full_packets >> 8) & 0xFF;  // Upper 8 bits of full packets
  buf[2] = remainder;                   // Remainder size
  if (SET_DEBUG) {
    Serial.printf("%d %d %d", buf[0], buf[1], buf[2]);
    for (int i = 3; i < PACKET_SIZE; i++) {
      buf[i] = 0;
      Serial.printf("%d ", buf[i]);
    }
  }
  Serial.println();
  bool sent = radio.write(buf, sizeof(buf));  // Send the buffer over radio
  if (sent) { return true; };
  return false;
}

// Function to send the end signal for image transmission
bool send_image_end() {
  uint8_t buf[PACKET_SIZE];
  for (int i = 0; i < PACKET_SIZE; i++) {
    buf[i] = 1;  // Fill the buffer with 1s
  }
  bool sent = radio.write(buf, sizeof(buf));  // Send the buffer over radio
  if (sent) { return true; };
  return false;
}

// Function to send the image over radio
void send_image(uint8_t* fb) {
  int start_time = millis();  // Record the start time

  String path = "/picture" + String(image_counter) + ".jpg";
  fs::FS& fs = SD;  // Reference to the file system

  File img = fs.open(path.c_str(), FILE_READ);

  if (img) {
    image_size = img.size();
    if (SET_DEBUG) { Serial.printf("Image size is %d bytes\n", image_size); };
    fb = new uint8_t[image_size * sizeof(uint8_t)];
    if (fb == NULL) {
      Serial.println("Could not allocate the memory.");
      delete[] fb;
      return;
    }
    // read until there's nothing else in it:
    while (img.available()) {
      img.read(fb, image_size);
    }

    Serial.printf("Number of packets to send is: %d\n", image_size / PACKET_SIZE);

    // close the file:
    img.close();
  } else {
    Serial.println("No image found.\n");
    return;
  };

  if (SET_DEBUG) {
    Serial.printf("Image size: %zu bytes\n", image_size);
  }

  if (send_image_start()) {
    Serial.println("The starting packet sent successfully");
    Serial.printf("The size of buffer: %d", image_size);
    if (send_image_credentials(image_size)) {
      Serial.println("The credentials packet sent successfully");
      packet_counter = 0;
      bool packet_ack;
      buf[PACKET_SIZE] = { 0 };  // Initialize a buffer with zeros
      for (int i = 0; i < image_size / PACKET_SIZE; i++) {
        for (int j = 0; j < PACKET_SIZE; j++) {
          buf[j] = fb[PACKET_SIZE * i + j];  // Fill the buffer with image data
          if (SET_DEBUG) {
            Serial.printf("%d ", buf[j]);
          }
        }
        if (SET_DEBUG) { Serial.println(); };

        packet_ack = radio.write(&buf, sizeof(buf));  // Send the buffer over radio
        if (SET_DEBUG) {
          if (packet_ack) {
            Serial.printf("Packet %d sent successfully.\n", packet_counter);
          } else {
            Serial.printf("Packet %d failed.\n", packet_counter);
          }
        }
        packet_counter++;
        delay(PACKETS_DELAY);  // Delay between packets
      }
      bool finished = send_image_end();  // Send the end signal
      if (finished) {
        Serial.println("The final packet was sent successfully");
      } else {
        Serial.println("The final packet failed");
      }
    } else {
      Serial.println("The credentials packet failed");
    }
  } else {
    Serial.println("The starting packet failed");
  }

  Serial.printf("The time taken is %3.3f seconds\n\n", (millis() - start_time) / 1000.0);
  // esp_camera_fb_return(fb);  // Return the buffer
  fb = NULL;
}

#endif /* ROUTINS_H */