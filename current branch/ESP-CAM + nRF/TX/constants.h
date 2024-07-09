#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <esp_now.h>     // main protocol
#include <WiFi.h>        // WIFI handler
#include <esp_wifi.h>    // WIFI for ESP boards
#include "esp_camera.h"  // camera initializer
#include "Arduino.h"
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"

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

// Pin definition for nRF24L01
#define NRF_CE_PIN 2 // 13
#define NRF_CSN_PIN 15
#define NRF_SCK_PIN 14
#define NRF_MISO_PIN 12 // 2
#define NRF_MOSI_PIN 13 // 4

// Pin definition for HC-SR501
#define HC_SR501_OUT_PIN 12

#define SET_DEBUG false

#define BAUDRATE 115200
#define DELAY_BETWEEN_IMAGES 10000  // seconds between taking two images
#define SETUP_DELAY 10000
#define PACKETS_DELAY 5

// NR
#define PACKET_SIZE 30
#define PIPE 0x1234567890LL

uint8_t* fb = nullptr;
long image_size = 0;
long packet_counter = 0;
uint8_t buf[PACKET_SIZE] = { 0 };
long failures = 0;

SPIClass *hspi = NULL;

#endif /* CONSTANTS_H */