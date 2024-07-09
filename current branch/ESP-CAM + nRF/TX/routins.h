// #include <sys/unistd.h>
// #include "Arduino.h"
// #include "esp_now.h"
// #include <sys/_stdint.h>
#include "HardwareSerial.h"
#include <nRF24L01.h>
#include <RF24.h>
#include <SPI.h>
#include "esp_camera.h"
#include "constants.h"
#ifndef ROUTINS_H
#define ROUTINS_H


RF24 radio(NRF_CE_PIN, NRF_CSN_PIN);  // CE and CS pin

camera_config_t camera_init() {
  // configure the camera
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
  config.pixel_format = PIXFORMAT_JPEG;  //YUV422,GRAYSCALE,RGB565,JPEG

  // select lower resolution if camera does not support PSRAM
  if (psramFound()) {
    /*
    possible framesizes that ESP32CAM supports: 
    FRAMESIZE_UXGA (1600 x 1200)
    FRAMESIZE_QVGA (320 x 240)
    FRAMESIZE_CIF (352 x 288)
    FRAMESIZE_VGA (640 x 480)
    FRAMESIZE_SVGA (800 x 600)
    FRAMESIZE_XGA (1024 x 768)
    FRAMESIZE_SXGA (1280 x 1024)
    */
    config.frame_size = FRAMESIZE_SVGA;  // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config.jpeg_quality = 10;            // 10 - about 65 kB, 12 - about 50 kB
    config.fb_count = 1;
  } else {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 15;
    config.fb_count = 1;
  }
  return config;
}

bool send_image_start() {
  uint8_t buf[PACKET_SIZE] = { 0 };
  bool sent = radio.write(buf, sizeof(buf));
  if (sent) { return true; };
  return false;
}

bool send_image_credentials(int size) {
  uint8_t buf[PACKET_SIZE];
  int full_packets = size / PACKET_SIZE;
  int remainder = size % PACKET_SIZE;

  buf[0] = full_packets & 0xFF;         // Lower 8 bits
  buf[1] = (full_packets >> 8) & 0xFF;  // Upper 8 bits
  buf[2] = remainder;                   // Remainder size
  Serial.printf("%d %d %d", buf[0], buf[1], buf[2]);
  for (int i = 3; i < PACKET_SIZE; i++) {
    buf[i] = 0;
    Serial.printf("%d ", buf[i]);
  }
  Serial.println();
  bool sent = radio.write(buf, sizeof(buf));
  if (sent) { return true; };
  return false;
}

bool send_image_end() {
  uint8_t buf[PACKET_SIZE];
  for (int i = 0; i < PACKET_SIZE; i++) {
    buf[i] = 1;
  }
  bool sent = radio.write(buf, sizeof(buf));
  if (sent) { return true; };
  return false;
}

void send_image(camera_fb_t* fb) {
  int start_time = millis();
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }
  image_size = fb->len;

  if (SET_DEBUG) {
    Serial.printf("Image size: %zu bytes\n", fb->len);
    Serial.printf("Image width: %d, height: %d\n", fb->width, fb->height);
  }

  if (send_image_start()) {
    Serial.println("The starting packet sent successfully");
    Serial.printf("The size of buffer: %d\n", image_size);
    if (send_image_credentials(fb->len)) {
      Serial.println("The credentials packet sent successfully");
      packet_counter = 0;
      bool packet_ack;
      buf[PACKET_SIZE] = { 0 };
      for (int i = 0; i < fb->len / PACKET_SIZE; i++) {
        // Serial.printf("Writing packet %d.\n", i);
        for (int j = 0; j < PACKET_SIZE; j++) {
          buf[j] = fb->buf[PACKET_SIZE * i + j];
          if (SET_DEBUG) {
            Serial.printf("%d ", buf[j]);
          }
        }
        if (SET_DEBUG) { Serial.println(); };

        // write the cur
        packet_ack = radio.write(&buf, sizeof(buf));
        if (SET_DEBUG) {
          if (packet_ack) {
            Serial.printf("Packet %d sent successfully.\n", packet_counter);
          } else {
            Serial.printf("Packet %d failed.\n", packet_counter);
          }
        }
        packet_counter++;
        delay(PACKETS_DELAY);
      }
      bool finished = send_image_end();
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
  // return the buffer when finished working with it
  esp_camera_fb_return(fb);
  fb = NULL;
}

// NRF ROUTINES
bool radio_init(SPIClass* hspi) {
  // if (!radio.isChipConnected()) {
  //   Serial.println("The nRF chip is not connected");
  // }
  // if (!radio.begin(hspi, NRF_CE_PIN, NRF_CSN_PIN)) {
  if (!radio.begin(hspi)) {
    Serial.println("Could not initialize radio");
    return false;
  } else {
    Serial.println("Radio initialized successfully");
  }
  // radio.begin();
  radio.setChannel(5);
  radio.setDataRate(RF24_1MBPS);
  radio.setPALevel(RF24_PA_HIGH);
  radio.openWritingPipe(PIPE);
  return true;
}

#endif /* ROUTINS_H */