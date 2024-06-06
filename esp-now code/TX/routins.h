// THIS IS A WORKING CODE

#include <sys/_stdint.h>
#include "HardwareSerial.h"
#include "esp_camera.h"
#ifndef ROUTINS_H
#define ROUTINS_H
#include "constants.h"

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
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 15;
    config.fb_count = 1;
  }
  return config;
}

// callback function when data is sent
void OnDataSent(const uint8_t* mac_addr, esp_now_send_status_t status) {
}

void register_peer() {
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

bool send_image_start(camera_fb_t* fb) {
  for (int i = 0; i < PACKET; ++i) {
    myData.arr[i] = 0;
  }
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*)&myData, sizeof(myData));
  if (result == ESP_OK) {
    Serial.println("Initial packet sent successfully");
    return true;
  } else {
    Serial.println("Initial packet failed");
    return false;
  }
}

bool send_fb_credentials(camera_fb_t* fb) {
  Serial.printf("Length if image buffer: %d\n", fb->len);
  Serial.printf("Amount of packets: %d \n", fb->len / PACKET);
  Serial.printf("Amount of bytes left : %d \n", fb->len % PACKET);
  if ((fb->len / PACKET) >= 255) {
    myData.arr[0] = 255;
    if (SET_DEBUG) { Serial.printf("%d ", myData.arr[0]); };
    myData.arr[1] = ((fb->len) / PACKET) - 255;
    if (SET_DEBUG) { Serial.printf("%d ", myData.arr[1]); };
    myData.arr[2] = (fb->len) % PACKET;
    if (SET_DEBUG) { Serial.printf("%d ", myData.arr[2]); };
  } else {
    myData.arr[0] = (fb->len) / PACKET;
    if (SET_DEBUG) { Serial.printf("%d ", myData.arr[0]); };
    myData.arr[1] = 0;
    if (SET_DEBUG) { Serial.printf("%d ", myData.arr[1]); };
    myData.arr[2] = (fb->len) % PACKET;
    if (SET_DEBUG) { Serial.printf("%d ", myData.arr[2]); };
  }
  for (int i = 3; i < PACKET; ++i) {
    myData.arr[i] = 0;
    if (SET_DEBUG) { Serial.printf("%d ", myData.arr[i]); };
  }
  if (SET_DEBUG) { Serial.println(); };
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*)&myData, sizeof(myData));
  if (result == ESP_OK) {
    Serial.println("Credentials packet sent successfully");
    return true;
  } else {
    Serial.println("Credentials packet failed");
    return false;
  }
  return true;
}

bool send_image_end(camera_fb_t* fb) {
  for (int i = 0; i < PACKET; ++i) {
    myData.arr[i] = 1;
  }
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*)&myData, sizeof(myData));
  if (result == ESP_OK) {
    Serial.println("Final packet sent successfully");
    return true;
  } else {
    Serial.println("Final packet failed");
    return false;
  }
}

void send_image(camera_fb_t* fb) {
  // Take Picture with Camera
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  } else {
    Serial.println("Image taken");
  }
  delay(50);
  // debug info
  if (SET_DEBUG) {
    Serial.printf("Image size: %zu bytes\n", fb->len);
    Serial.printf("Image width: %d, height: %d\n", fb->width, fb->height);
    // Serial.println(fb->format);
  }

  // initialize parameters
  int num_packets = 1;
  int num_failures = 0;

  if (!send_image_start(fb)) {
    Serial.println("Send failed - could not send the initializer packet");
    return;
  } else {
    send_fb_credentials(fb);
    // Serial.println("The credentials sent successfully");
    int rows = 0;
    for (int j = 0; j < (fb->len / PACKET); ++j) {  // go over framebuffer in packets of 200
      for (int i = 0; i < PACKET; ++i) {
        myData.arr[i] = fb->buf[j * PACKET + i];
        if (SET_DEBUG) {
          Serial.printf("%d ", myData.arr[i]);  // print the array values
        }
      }
      if (SET_DEBUG) { Serial.println(); };
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*)&myData, sizeof(myData));
      if (result == ESP_OK) {
        if (SET_DEBUG) {
          Serial.printf("Packet %d sent successfully.\n", num_packets);
        }
      } else {
        num_failures++;
        if (SET_DEBUG) {
          Serial.printf("Packet %d failed.\n", num_packets);
        }
      }
      delay(50);  // give small delay between packets - CHECK IF NEEDED
      rows++;
      num_packets++;
    }
    for (int i = 0; i < sizeof(myData.arr); ++i) {
      myData.arr[i] = 0;
    }

    // MODULO calculation
    int cnt = 0;
    Serial.printf("Sizeof buffer before modulo: %d and sizeof modulo: %d\n", fb->len - (fb->len % PACKET), (fb->len % PACKET));
    for (int i = 0; i < (fb->len % PACKET); ++i) {
      myData.arr[i] = fb->buf[num_packets * PACKET + i];
      // Serial.printf("%d ", fb->buf[num_packets * PACKET + i]);
      Serial.printf("%d ", myData.arr[i]);
      cnt++;
    }
    myData.arr[cnt + 1] = 255;
    Serial.printf("%d ", myData.arr[cnt + 1]);
    myData.arr[cnt + 2] = 217;
    Serial.printf("%d ", myData.arr[cnt + 2]);
    for (int i = cnt + 3; i < PACKET; i++) {
      // myData.arr[i] = 0;
      Serial.printf("%d ", myData.arr[i]);
    }
    if (SET_DEBUG) { Serial.println(); };

    // Serial.println("myData.arr:");
    // for (int i = 0; i < PACKET; i++) {
    //   Serial.printf("%d ", myData.arr[i]);
    // }
    // Serial.println();

    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*)&myData, sizeof(myData));
    if (result == ESP_OK) {
      Serial.printf("Modulo sent successfully.\n");
    } else {
      num_failures++;
      Serial.printf("Modulo failed.\n");
    }

    // debug info
    if (SET_DEBUG) {
      Serial.printf("Image size : %d\n", fb->len);
      Serial.printf("Packet size: %d\n", sizeof(myData));
      Serial.printf("Number of failed packages: %d\n", num_failures);
    }
  }
  send_image_end(fb);
  // return the buffer when finished working with it
  esp_camera_fb_return(fb);
  fb = NULL;
}

#endif /* ROUTINS_H */