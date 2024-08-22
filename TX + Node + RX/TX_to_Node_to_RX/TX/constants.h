#ifndef CONSTANTS_H
#define CONSTANTS_H

// Include necessary libraries
#include "esp_camera.h"  // Camera initializer
#include "FS.h"                // File system for SD Card on ESP32
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "driver/rtc_io.h"     // RTC I/O driver
#include "HardwareSerial.h"    // Include the HardwareSerial library for serial communication
#include <nRF24L01.h>          // Include the nRF24L01 library for radio communication
#include <RF24.h>              // Include the RF24 library for handling nRF24L01
#include <SPI.h>               // Include the SPI library for SPI communication


// Pin definition for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

// Pin definition for nRF24L01 radio module
#define NRF_CE_PIN 2     // Chip Enable pin
#define NRF_CSN_PIN 15   // Chip Select Not pin
#define NRF_SCK_PIN 14   // Serial Clock pin
#define NRF_MISO_PIN 12  // Master In Slave Out pin
#define NRF_MOSI_PIN 13  // Master Out Slave In pin

// Pin definition for HC-SR501 motion sensor
#define PIR_PIN 4  // Output pin for the motion sensor

// Debugging settings
#define SET_DEBUG false  // Debugging flag

// Serial communication settings
#define BAUDRATE 115200  // Baud rate for serial communication

// Timing settings
#define DELAY_BETWEEN_IMAGES 10000  // Delay between capturing two images in milliseconds
#define SETUP_DELAY 10000           // Setup delay in milliseconds
#define PACKETS_DELAY 0.1           // Delay between packet transmissions in milliseconds

// nRF24L01 radio module settings
#define PACKET_SIZE 30       // Size of each packet in bytes
#define PIPE 0x1234567890LL  // Address of the communication pipe

// Global variables
uint8_t* fb = nullptr;             // Pointer to the frame buffer
long image_size = 0;               // Size of the image
long packet_counter = 0;           // Counter for packets
uint8_t buf[PACKET_SIZE] = { 0 };  // Buffer for packets
long failures = 0;                 // Counter for failures

SPIClass* hspi = NULL;  // Pointer to the HSPI instance

#endif /* CONSTANTS_H */
