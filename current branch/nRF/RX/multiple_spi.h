#ifndef MULTIPLESPI_H
#define MULTIPLESPI_H

// // Pin definition for CAMERA_MODEL_AI_THINKER
// #define PWDN_GPIO_NUM 32
// #define RESET_GPIO_NUM -1
// #define XCLK_GPIO_NUM 0
// #define SIOD_GPIO_NUM 26
// #define SIOC_GPIO_NUM 27
// #define Y9_GPIO_NUM 35
// #define Y8_GPIO_NUM 34
// #define Y7_GPIO_NUM 39
// #define Y6_GPIO_NUM 36
// #define Y5_GPIO_NUM 21
// #define Y4_GPIO_NUM 19
// #define Y3_GPIO_NUM 18
// #define Y2_GPIO_NUM 5
// #define VSYNC_GPIO_NUM 25
// #define HREF_GPIO_NUM 23
// #define PCLK_GPIO_NUM 22

#include <SPI.h>

// Define ALTERNATE_PINS to use non-standard GPIO pins for SPI bus


#define VSPI_MISO 19
#define VSPI_MOSI 23
#define VSPI_SCLK 18
#define VSPI_SS 5

#define HSPI_MISO 12
#define HSPI_MOSI 13
#define HSPI_SCLK 14
#define HSPI_SS 15

//uninitialised pointers to SPI objects
SPIClass *vspi = NULL;
SPIClass *hspi = NULL;

#endif /* MULTIPLESPI_H */