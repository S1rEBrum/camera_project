#include "constants.h"  // Including a header file that defines constants used in this file
#ifndef ROUTINS_H
#define ROUTINS_H

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

// Function to initialize the radio module
void init_radio() {
  // Attempt to initialize the radio with HSPI
  if (!radio.begin(hspi)) {
    Serial.println("Could not initialize radio");  // Print error message if initialization fails
    ESP.restart();
    return;
  } else {
    Serial.println("\n\nRadio initialized successfully");  // Print success message if initialization is successful
  }
  radio.setChannel(100);           // Set the radio channel to 5
  radio.setDataRate(RF24_1MBPS);   // Set the data rate to 1Mbps
  radio.setPALevel(RF24_PA_MAX);   // Set the power amplifier level to high
  radio.openReadingPipe(1, PIPE);  // Open a reading pipe with a specific address (PIPE)
  radio.startListening();          // Start listening for incoming data
}

// Function to initialize the SD card
void init_sd() {
  // Attempt to initialize the SD card with VSPI and its SS pin
  if (!SD.begin(VSPI_SS, *vspi)) {
    Serial.println("SD Card Mount Failed");  // Print error message if initialization fails
    ESP.restart();
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

// function to clear SD card
void delete_files_on_sd() {
  File dir = SD.open("/");
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) {
      break;
    }
    String fileName = entry.name();
    if (fileName.endsWith(".jpg")) {
      int removed = SD.remove("/" + fileName);
      if (SET_DEBUG) {
        if (removed) {
          Serial.print("Deleted: ");
          Serial.println(fileName);
        }
      }
    }
    entry.close();
  }
  dir.close();
  Serial.println();
}

// Function to check if the start of an image has been received
bool got_image_start(uint8_t buffer[]) {
  // Check if all bytes in the buffer are 0
  for (int i = 0; i < PACKET_SIZE; i++) {
    if (buffer[i] != 0) {
      return false;  // Return false if any byte is not 0
    }
  }
  return true;  // Return true if all bytes are 0
}

// Function to get the image size from the buffer
int got_image_credentials(uint8_t buffer[]) {
  // Calculate the number of full packets and the remainder from the buffer
  int full_packets = buffer[0] + (buffer[1] << 8);
  int remainder = buffer[2];
  int image_size = full_packets * PACKET_SIZE + remainder;

  if (SET_DEBUG) {
    Serial.printf("The size of the received image: %d\n", image_size);  // Print the size of the received image if debugging is enabled
  }
  return image_size;  // Return the image size
}

// Function to check if the end of an image has been received
bool got_image_end(uint8_t buffer[]) {
  // Check if all bytes in the buffer are 1
  for (int i = 0; i < PACKET_SIZE; i++) {
    if (buffer[i] != 1) {
      return false;  // Return false if any byte is not 1
    }
  }
  return true;  // Return true if all bytes are 1
}

// Function to save an image to the SD card
void save_image_to_sd(uint8_t *fb, int size) {
  if (size <= 0) {
    Serial.println("No correct data to save");
    return;
  }
  Serial.printf("Starting the writing to SD");

  // Create the file path for the image
  String path = "/picture" + String(image_counter) + ".jpg";

  fs::FS &fs = SD;  // Reference to the file system

  File file = fs.open(path.c_str(), FILE_WRITE);  // Open the file in write mode
  if (!file) {
    Serial.println("Failed to open file in writing mode\n");  // Print error message if the file cannot be opened
  } else {
    file.write(fb, size);  // Write the image data to the file
    for (int i = 0; i < size; ++i) {
      Serial.printf("%d ", fb[i]);
    }
    // Serial.print(fb, size);
    uint8_t n1 = 255;  // JPEG end-of-image marker byte 1
    uint8_t n2 = 217;  // JPEG end-of-image marker byte 2
    file.write(n1);    // Write the end-of-image marker byte 1
    file.write(n2);    // Write the end-of-image marker byte 2
    Serial.printf("%d ", n1);
    Serial.printf("%d ", n2);
    Serial.print("\n");
    Serial.printf("Saved file to path: %s\n\n", path.c_str());  // Print the path of the saved file
  }
  file.close();     // Close the file
  image_counter++;  // Increment the image counter
}

void receive_image() {
  // Check if there is data available to read from the radio
  if (radio.available()) {
    // Read the data into the buffer
    radio.read(buf, PACKET_SIZE);

    // If debugging is enabled, print the packet contents
    if (SET_DEBUG) {
      Serial.printf("Packet %d: ", packet_counter);
      for (int i = 0; i < PACKET_SIZE; ++i) {
        Serial.printf("%d ", buf[i]);
      }
      Serial.println();
    }

    // Check if the end of the image has been received
    if (got_image_end(buf)) {
      Serial.println("Got end packet");
      // if the amount of images on the SD reached N - delete all
      if (image_counter > MAX_IMAGES_TO_SAVE) {
        delete_files_on_sd();
        image_counter = 0;
      }
      // Save the image to the SD card
      save_image_to_sd(fb, image_size);
      delete[] fb;                    // Free the allocated memory for the image
      image_size = 0;                 // Reset the image size
      packet_counter = 0;             // Reset the packet counter
      receiving_image = false;        // Reset the receiving image flag
      receiving_credentials = false;  // Reset the receiving credentials flag
    }
    // Check if image data is being received
    else if (receiving_image) {
      // Copy the received packet data to the frame buffer
      if (image_size - packet_counter * PACKET_SIZE >= PACKET_SIZE) {
        for (int i = 0; i < PACKET_SIZE; i++) {
          fb[packet_counter * PACKET_SIZE + i] = buf[i];
        }
      } else {
        int modulo_size = image_size - packet_counter * PACKET_SIZE;
        for (int i = 0; i < modulo_size; i++) {
          fb[(packet_counter)*PACKET_SIZE + i] = buf[i];
        }
      }
      packet_counter++;  // Increment the packet counter
    }
    // Check if image credentials are being received
    else if (receiving_credentials) {
      // Get the image size from the received credentials
      image_size = got_image_credentials(buf);
      Serial.printf("Got credentials, the image size is: %d\n", image_size);
      if (SET_DEBUG) {
        Serial.printf("Got image size of %d bytes\n", image_size);
      }
      // Allocate memory for the image frame buffer
      fb = new uint8_t[image_size];
      if (fb == NULL) {
        Serial.println("Could not allocate memory");
        return;
      }
      receiving_credentials = false;  // Reset the receiving credentials flag
      receiving_image = true;         // Set the receiving image flag
    }
    // Check if the start of the image has been received
    else if (got_image_start(buf)) {
      Serial.println("Got start packet");
      receiving_credentials = true;  // Set the receiving credentials flag
      packet_counter = 0;            // Reset the packet counter
    }
  }
}

#endif /* ROUTINS_H */
