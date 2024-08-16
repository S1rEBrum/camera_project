#ifndef MULTIPLESPI_H
#define MULTIPLESPI_H

#include <SPI.h>  // Include the SPI library

// Define pin mappings for VSPI (SPI2) interface
#define VSPI_MISO 19  // MISO pin for VSPI
#define VSPI_MOSI 23  // MOSI pin for VSPI
#define VSPI_SCLK 18  // SCLK pin for VSPI
#define VSPI_SS 5     // SS pin for VSPI

// Define pin mappings for HSPI (SPI3) interface
#define HSPI_MISO 12  // MISO pin for HSPI
#define HSPI_MOSI 13  // MOSI pin for HSPI
#define HSPI_SCLK 14  // SCLK pin for HSPI
#define HSPI_SS 15    // SS pin for HSPI

// Declare pointers for SPIClass instances
SPIClass *vspi = NULL;  // Pointer for VSPI instance
SPIClass *hspi = NULL;  // Pointer for HSPI instance

#endif /* MULTIPLESPI_H */
