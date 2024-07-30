#include "constants.h"  // Including a header file that defines constants used in this file
#ifndef GLOBAL_ROUTINS_H
#define GLOBAL_ROUTINS_H

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

void delete_files_on_sd() {
  File dir = SD.open("/");
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) {
      break;
    }
    String fileName = entry.name();
    if (fileName.endsWith(".jpg")) {
      SD.remove(fileName);
      if (SET_DEBUG) {
        Serial.print("Deleting: ");
        Serial.println(fileName);
      }
    }
    entry.close();
  }
  dir.close();
  Serial.println();
}

#endif /* GLOBAL_ROUTINS_H */