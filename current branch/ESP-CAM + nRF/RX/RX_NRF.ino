//---------------------------------------
// RX MODULE
//---------------------------------------
#include "routins.h"  // Include the routines header file

void setup() {
  // Initialize multiple SPI interfaces
  init_spi();
  delay(1000);  // Delay to allow for proper initialization

  // Begin serial communication at the defined baud rate
  Serial.begin(BAUDRATE);

  // Initialize the RF module
  init_radio();

  // Initialize the microSD card module
  init_sd();
}

void loop() {
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
      Serial.println("Got start");
      receiving_credentials = true;  // Set the receiving credentials flag
      packet_counter = 0;            // Reset the packet counter
    }
  }
}
