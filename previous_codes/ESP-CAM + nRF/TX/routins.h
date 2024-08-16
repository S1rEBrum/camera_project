#include "HardwareSerial.h"  // Include the HardwareSerial library for serial communication
#include <nRF24L01.h>        // Include the nRF24L01 library for radio communication
#include <RF24.h>            // Include the RF24 library for handling nRF24L01
#include <SPI.h>             // Include the SPI library for SPI communication
#include "esp_camera.h"      // Include the ESP32 camera library
#include "constants.h"       // Include a user-defined constants file (assumed to be provided)
#ifndef ROUTINS_H
#define ROUTINS_H

// Initialize the RF24 radio object with specified CE and CS pins
RF24 radio(NRF_CE_PIN, NRF_CSN_PIN);

// Function to configure and initialize the camera
camera_config_t camera_init() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;  // Set pixel format to JPEG

  // Select lower resolution if camera does not support PSRAM
  if (psramFound()) {
    config.frame_size = FRAMESIZE_SVGA;  // Set frame size to SVGA if PSRAM is found
    config.jpeg_quality = 10;            // Set JPEG quality
    config.fb_count = 1;
  } else {
    config.frame_size = FRAMESIZE_VGA;  // Set frame size to VGA if PSRAM is not found
    config.jpeg_quality = 15;           // Set JPEG quality
    config.fb_count = 1;
  }
  return config;
}

// Function to send the start signal for image transmission
bool send_image_start() {
  uint8_t buf[PACKET_SIZE] = { 0 };           // Initialize a buffer with zeros
  bool sent = radio.write(buf, sizeof(buf));  // Send the buffer over radio
  if (sent) { return true; };
  return false;
}

// Function to send image size and packet credentials
bool send_image_credentials(int size) {
  uint8_t buf[PACKET_SIZE];
  int full_packets = size / PACKET_SIZE;
  int remainder = size % PACKET_SIZE;

  buf[0] = full_packets & 0xFF;         // Lower 8 bits of full packets
  buf[1] = (full_packets >> 8) & 0xFF;  // Upper 8 bits of full packets
  buf[2] = remainder;                   // Remainder size
  if (SET_DEBUG) {
    Serial.printf("%d %d %d", buf[0], buf[1], buf[2]);
    for (int i = 3; i < PACKET_SIZE; i++) {
      buf[i] = 0;
      Serial.printf("%d ", buf[i]);
    }
  }
  Serial.println();
  bool sent = radio.write(buf, sizeof(buf));  // Send the buffer over radio
  if (sent) { return true; };
  return false;
}

// Function to send the end signal for image transmission
bool send_image_end() {
  uint8_t buf[PACKET_SIZE];
  for (int i = 0; i < PACKET_SIZE; i++) {
    buf[i] = 1;  // Fill the buffer with 1s
  }
  bool sent = radio.write(buf, sizeof(buf));  // Send the buffer over radio
  if (sent) { return true; };
  return false;
}

// Function to send the image over radio
void send_image(camera_fb_t* fb) {
  int start_time = millis();  // Record the start time
  fb = esp_camera_fb_get();   // Capture an image with the camera
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }
  image_size = fb->len;  // Get the image size

  if (SET_DEBUG) {
    Serial.printf("Image size: %zu bytes\n", fb->len);
    Serial.printf("Image width: %d, height: %d\n", fb->width, fb->height);
  }

  if (send_image_start()) {
    Serial.println("The starting packet sent successfully");
    Serial.printf("The size of buffer: %d", image_size);
    if (send_image_credentials(fb->len)) {
      Serial.println("The credentials packet sent successfully");
      packet_counter = 0;
      bool packet_ack;
      buf[PACKET_SIZE] = { 0 };  // Initialize a buffer with zeros
      for (int i = 0; i < fb->len / PACKET_SIZE; i++) {
        for (int j = 0; j < PACKET_SIZE; j++) {
          buf[j] = fb->buf[PACKET_SIZE * i + j];  // Fill the buffer with image data
          if (SET_DEBUG) {
            Serial.printf("%d ", buf[j]);
          }
        }
        if (SET_DEBUG) { Serial.println(); };

        packet_ack = radio.write(&buf, sizeof(buf));  // Send the buffer over radio
        if (SET_DEBUG) {
          if (packet_ack) {
            Serial.printf("Packet %d sent successfully.\n", packet_counter);
          } else {
            Serial.printf("Packet %d failed.\n", packet_counter);
          }
        }
        packet_counter++;
        delay(PACKETS_DELAY);  // Delay between packets
      }
      bool finished = send_image_end();  // Send the end signal
      if (finished) {
        Serial.println("The final packet was sent successfully");
      } else {
        Serial.println("The final packet failed");
      }
    } else {
      Serial.println("The credentials packet failed");
    }
  } else {
    Serial.println("The starting packet failed");
  }

  Serial.printf("The time taken is %3.3f seconds\n\n", (millis() - start_time) / 1000.0);
  esp_camera_fb_return(fb);  // Return the buffer
  fb = NULL;
}

// Function to initialize the RF24 radio
bool radio_init(SPIClass* hspi) {
  if (!radio.begin(hspi)) {
    Serial.println("Could not initialize radio");
    return false;
  } else {
    Serial.println("Radio initialized successfully");
  }
  radio.setChannel(5);             // Set the radio channel
  radio.setDataRate(RF24_1MBPS);   // Set the data rate
  radio.setPALevel(RF24_PA_HIGH);  // Set the power amplifier level
  radio.openWritingPipe(PIPE);     // Open the writing pipe
  return true;
}

#endif /* ROUTINS_H */
