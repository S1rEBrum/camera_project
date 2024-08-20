#include "RF24.h"
#include "constants.h"  // Including a header file that defines constants used in this file
#include "tx_routins.h"
#ifndef RX_ROUTINS_H
#define RX_ROUTINS_H
//------------------------------------------------------------------------------------------------------------------------
// RECEIVING IMAGE ROUTINES
//------------------------------------------------------------------------------------------------------------------------

// Function to initialize the radio module
void init_rx_radio() {
  // Attempt to initialize the radio with HSPI
  if (!radio.begin(hspi)) {
    Serial.println("\nCould not initialize RX radio");  // Print error message if initialization fails
    ESP.restart();
    return;
  } else {
    Serial.println("\nRX Radio initialized successfully");  // Print success message if initialization is successful
  }
  radio.setChannel(25);                // Set the radio channel to 5
  radio.setDataRate(RF24_2MBPS);      // Set the data rate to 1Mbps
  radio.setPALevel(RF24_PA_LOW);      // Set the power amplifier level to high
  radio.openReadingPipe(1, RX_PIPE);  // Open a reading pipe with a specific address (PIPE)
  radio.startListening();             // Start listening for incoming data
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
bool save_image_to_sd(uint8_t* fb, int size) {
  if (size <= 0) {
    Serial.println("No correct data to save\n");
    return false;
  }
  Serial.println("Starting the writing to SD");

  // Create the file path for the image
  String path = "/picture" + String(image_counter) + ".jpg";

  fs::FS& fs = SD;  // Reference to the file system

  File file = fs.open(path.c_str(), FILE_WRITE);  // Open the file in write mode
  if (!file) {
    Serial.println("Failed to open file in writing mode\n");  // Print error message if the file cannot be opened
  } else {
    file.write(fb, size);                                       // Write the image data to the file
    uint8_t n1 = 255;                                           // JPEG end-of-image marker byte 1
    uint8_t n2 = 217;                                           // JPEG end-of-image marker byte 2
    file.write(n1);                                             // Write the end-of-image marker byte 1
    file.write(n2);                                             // Write the end-of-image marker byte 2
    Serial.printf("Saved file to path: %s\n\n", path.c_str());  // Print the path of the saved file
  }
  file.close();     // Close the file
  image_counter++;  // Increment the image counter
  return true;
}

// main function for receiving the images
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
      // Save the image to the SD card
      bool saved = save_image_to_sd(fb, image_size);
      // delete[] fb;                    // Free the allocated memory for the image
      if (saved) {
        Serial.println("Finished getting the image. Start sending now.");
        init_tx_radio();  // need to add it!!
        bool sent_successfully = send_image(fb, image_size);
        if (sent_successfully) {
          Serial.println("Image sent successfully.");
        } else {
          Serial.println("Sending image failed.");
        }
      }
      image_size = 0;                 // Reset the image size
      packet_counter = 0;             // Reset the packet counter
      receiving_image = false;        // Reset the receiving image flag
      receiving_credentials = false;  // Reset the receiving credentials flag
      init_rx_radio();
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

#endif /* RX_ROUTINS_H */