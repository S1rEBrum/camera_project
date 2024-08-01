#include "constants.h"  // Including a header file that defines constants used in this file
#ifndef TX_ROUTINS_H
#define TX_ROUTINS_H

//------------------------------------------------------------------------------------------------------------------------
// SENDING IMAGE ROUTINES
//------------------------------------------------------------------------------------------------------------------------

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


// Function to initialize the RF24 radio
void init_tx_radio() {
  if (!radio.begin(hspi)) {
    Serial.println("\nCould not initialize TX radio");
    return;
  } else {
    Serial.println("\nTX Radio initialized successfully");
  }
  radio.setChannel(5);             // Set the radio channel
  radio.setDataRate(RF24_1MBPS);   // Set the data rate
  radio.setPALevel(RF24_PA_HIGH);  // Set the power amplifier level
  radio.openWritingPipe(TX_PIPE);  // Open the writing pipe
  return;
}

// main function to send the image
bool send_image(uint8_t* fb, int image_size) {
  int start_time = millis();  // Record the start time

  // String path = "/picture" + String(counter) + ".jpg";
  // fs::FS& fs = SD;  // Reference to the file system

  // File img = fs.open(path.c_str(), FILE_READ);

  // if (img) {
  //   image_size = img.size();
  //   if (SET_DEBUG) { Serial.printf("Image size is %d bytes\n", image_size); };
  //   fb = new uint8_t[image_size * sizeof(uint8_t)];
  //   if (fb == NULL) {
  //     Serial.println("Could not allocate the memory.");
  //     delete[] fb;
  //     return;
  //   }
  //   // read until there's nothing else in it:
  //   while (img.available()) {
  //     img.read(fb, image_size);
  //   }
  //   Serial.printf("The image size is: %d\n", image_size);
  //   Serial.printf("Number of packets to send is: %d\n", image_size / PACKET_SIZE);

  //   // close the file:
  //   img.close();
  // } else {
  //   Serial.println("No image found.\n");
  //   return;
  // };
  // image_size = sizeof(fb);
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
        return false;
      }
    } else {
      Serial.println("The credentials packet failed");
      return false;
    }
  } else {
    Serial.println("The starting packet failed");
    return false;
  }

  Serial.printf("The time taken is %3.3f seconds\n\n", (millis() - start_time) / 1000.0);
  // esp_camera_fb_return(fb);  // Return the buffer
  delete[] fb;
  fb = NULL;
  return true;
}

#endif /* TX_ROUTINS_H */