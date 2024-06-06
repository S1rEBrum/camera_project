// THIS IS A WORKING CODE

#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "esp_camera.h"

#define PACKET 200

typedef struct struct_message {
  uint8_t arr[PACKET];
} struct_message;

struct_message myData;

int image_size;
int packet_counter = 0;
bool receiving_image = false;
bool start_writing = false;
bool receive_credentials = false;
int pictureNumber = 0;
uint8_t *fb = NULL;  // Frame buffer pointer

bool received_image_start() {
  for (int i = 0; i < PACKET; i++) {
    if (myData.arr[i] != 0) {
      return false;
    }
  }
  return true;
}

uint8_t *received_fb_credentials(uint8_t *fb) {
  image_size = (myData.arr[0] + myData.arr[1]) * PACKET + myData.arr[2];
  Serial.printf("The size of the received image: %d\n", image_size);
  uint8_t *new_fb = (uint8_t *)realloc(fb, image_size * sizeof(uint8_t));
  if (new_fb == NULL) {
    free(fb);
    return NULL;
  }
  for (int i = 0; i < sizeof(fb); ++i) {
    fb[i] = 0;
  }
  return new_fb;
}

bool received_image_end() {
  for (int i = 0; i < PACKET; i++) {
    if (myData.arr[i] != 1) {
      return false;
    }
  }
  return true;
}

void write_file(uint8_t *fb) {
  Serial.println("Starting the writing to SD");

  String path = "/picture" + String(pictureNumber) + ".jpg";

  fs::FS &fs = SD;
  Serial.printf("Picture file name: %s\n", path.c_str());

  File file = fs.open(path.c_str(), FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file in writing mode");
  } else {
    Serial.printf("Size of frame buffer is : %d\n", image_size);
    file.write(fb, image_size);
    // uint8_t n1 = 255;
    // uint8_t n2 = 217;
    // file.write(n1);
    // file.write(n2);
    Serial.printf("Saved file to path: %s\n", path.c_str());
  }

  file.close();
  for (int i = 0; i < sizeof(fb); ++i) {
    fb[i] = 0;
  }
  pictureNumber++;
}

void OnDataRecv(const esp_now_recv_info *mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  // Serial.printf("Bytes received: %d\n", len);
  // Serial.print("Array: ");
  // for (int i = 0; i < sizeof(myData.arr); ++i) {
  //   Serial.printf("%d ", myData.arr[i]);
  // }
  // Serial.println("");

  if (received_image_end()) {
    Serial.printf("Received packages: %d\n", packet_counter);
    Serial.printf("Image size: %d\n", image_size);
    write_file(fb);
    free(fb);
    fb = NULL;
    // for (int i=0; i<sizeof(myData.arr);++i) {
    //   myData.arr[i] =0;
    // }
    packet_counter = 0;
    receiving_image = false;
    receive_credentials = false;
    image_size = 0;
  } else if (receiving_image) {
    if (image_size - packet_counter * PACKET >= PACKET) {
      for (int i = 0; i < PACKET; i++) {
        fb[packet_counter * PACKET + i] = myData.arr[i];
      }
    } else {
      int modulo_size = image_size - packet_counter * PACKET;
      Serial.printf("Modulo size: %d\n", modulo_size);
      for (int i = 0; i < modulo_size; i++) {
        fb[(packet_counter)*PACKET + i] = myData.arr[i];
      }
    }
    packet_counter++;
  } else if (receive_credentials) {
    fb = received_fb_credentials(fb);
    // Serial.printf("Sizeof frame buffer after realloc: %d\n", sizeof(*fb));
    if (fb == NULL) {
      Serial.println("There was a realloc error");
      return;
    }
    receive_credentials = false;
    receiving_image = true;
  } else if (received_image_start()) {
    receive_credentials = true;
    if (fb == NULL) {
      fb = (uint8_t *)malloc(PACKET * sizeof(uint8_t));
      if (fb == NULL) {
        Serial.println("Initial malloc failed");
        return;
      }
    }
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_MODE_STA);
  esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR);
  esp_wifi_config_espnow_rate(WIFI_IF_STA, WIFI_PHY_RATE_1M_L);


  WiFi.disconnect();

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);

  Serial.println("Starting SD Card...");
  if (!SD.begin()) {
    Serial.println("SD Card Mount Failed");
    return;
  } else {
    Serial.println("SD card initialized successfully");
  }

  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD Card attached");
    return;
  }
}

void loop() {
  // Empty loop
}