//---------------------------------------
// NODE RECEIVER + TRANSMITTER
//---------------------------------------

// Include the routines header files
#include "global_routins.h"
// #include "tx_routins.h"
#include "rx_routins.h"


void setup() {
  // Initialize multiple SPI interfaces
  init_spi();
  delay(1000);  // Delay to allow for proper initialization

  // Begin serial communication at the defined baud rate
  Serial.begin(BAUDRATE);

  // Initialize the RF module
  init_rx_radio();

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
