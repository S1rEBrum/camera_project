#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "multiple_spi.h"  // Include header for multiple SPI configuration
#include <nRF24L01.h>      // Include header for nRF24L01 radio module
#include <RF24.h>          // Include header for RF24 library
#include <SD.h>            // Include header for SD card library

// Define constants for the project
#define BAUDRATE 115200      // Baud rate for serial communication
#define SET_DEBUG false      // Debugging flag
#define PACKET_SIZE 30       // Size of each packet
#define PIPE 0x0987654321LL  // Address of the radio communication pipe

// Declare global variables
uint8_t *fb = nullptr;             // Frame buffer for image data
uint8_t buf[PACKET_SIZE] = { 0 };  // Buffer for receiving data packets
int image_size = 0;                // Variable to store the size of the received image
int packet_counter = 0;            // Counter for the number of packets received
int image_counter = 0;             // Counter for the number of images saved

// Flags for image reception state
bool receiving_image = false;        // Flag to indicate if an image is being received
bool receiving_credentials = false;  // Flag to indicate if image credentials are being received

// Initialize the RF24 radio with specified CE and CSN pins
RF24 radio(4, 15);

#endif /* CONSTANTS_H */
