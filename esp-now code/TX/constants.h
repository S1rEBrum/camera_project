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
#include <esp_pm.h>
#include <esp_wifi.h>
#include <esp_wifi_types.h>

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

// broadcast address
// uint8_t broadcastAddress[] = { 0x78, 0xe3, 0x6d, 0x1a, 0xae, 0xb8 };
uint8_t broadcastAddress[] = { 0x78, 0xe3, 0x6d, 0x1a, 0xb3, 0xbc };


#define SET_DEBUG false

#define PACKET 200  //total amount of bytes we transmit in each packet
#define BAUDRATE 115200
#define DELAY_BETWEEN_IMAGES 2000  // seconds between taking two images

// Structure to send data
// Must match the receiver structure
typedef struct struct_message {
  uint8_t arr[PACKET];
} struct_message;

// struct for data
struct_message myData;

// struct for peer info
esp_now_peer_info_t peerInfo;

#endif /* CONSTANTS_H */