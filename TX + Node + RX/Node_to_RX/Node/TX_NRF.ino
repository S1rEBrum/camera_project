//---------------------------------------
// TX MODULE
//---------------------------------------
#include "routins.h"

void setup() {
  // Initialize the serial communication
  init_spi();

  delay(1000);

  // Begin serial communication at the defined baud rate
  Serial.begin(BAUDRATE);

  // Initialize the RF module
  init_radio();

  // Initialize the microSD card module
  init_sd();

  delay(SETUP_DELAY);
}

void loop() {
  // send an image from the framebuffer
  send_image(fb);

  // update image counter
  image_counter += 1;

  // put a delay before taking the new image
  delay(DELAY_BETWEEN_IMAGES);
}