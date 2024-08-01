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

  // clean the SD
  delete_files_on_sd();

  // reset the image counter for saving pictures
  image_counter = 0;
}

void loop() {
  receive_image();
}
