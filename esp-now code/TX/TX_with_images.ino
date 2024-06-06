#include "routins.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "esp_camera.h"
#include "driver/rtc_io.h"

// Define the maximum transmission power value (84 corresponds to 21 dBm)
#define MAX_TX_POWER 84

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);  // disable brownout detector

  Serial.begin(BAUDRATE);  // initialize serial monitor

  // initialize the camera configuration
  camera_config_t config = camera_init();

  // initialize the camera itself
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_MODE_STA);
  esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR);  // enable long range protocol

  // Set the maximum transmission power
  esp_wifi_set_max_tx_power(MAX_TX_POWER);
  esp_wifi_config_espnow_rate(WIFI_IF_STA, WIFI_PHY_RATE_1M_L);

  // disable power saving - check if needed
  esp_wifi_set_ps(WIFI_PS_NONE);
  WiFi.disconnect();

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Transmitted packet
  esp_now_register_send_cb(OnDataSent);

  // register the peer with the given broadcast address
  register_peer();

  // give some delay to initialization
  delay(5000);
}

void loop() {
  // initialize the frame buffer with NULL
  camera_fb_t *fb = NULL;
  unsigned long start_time = millis();
  // send an image from the framebuffer
  send_image(fb);
  unsigned long end_time = millis();
  Serial.println();
  Serial.printf("The time taken: %d\n", (end_time - start_time));
  Serial.println();

  // return the buffer when finished working with it
  esp_camera_fb_return(fb);

  // put a delay before taking the new image
  delay(DELAY_BETWEEN_IMAGES);
}