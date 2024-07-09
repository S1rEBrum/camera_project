// #include "multiple_spi.h"
#include "routins.h"

void setup() {
  // Initialize nRF24L01 pins
  pinMode(NRF_CE_PIN, OUTPUT);
  pinMode(NRF_CSN_PIN, OUTPUT);
  pinMode(NRF_SCK_PIN, OUTPUT);
  pinMode(NRF_MISO_PIN, INPUT);
  pinMode(NRF_MOSI_PIN, OUTPUT);

  digitalWrite(NRF_CSN_PIN, HIGH);
  digitalWrite(NRF_MISO_PIN, LOW);

  // Initialize HC-SR501 pin
  pinMode(HC_SR501_OUT_PIN, INPUT);

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);  //disable brownout detector
  Serial.begin(BAUDRATE);


  hspi = new SPIClass(HSPI);
  // hspi->begin();
  hspi->begin(NRF_SCK_PIN, NRF_MISO_PIN, NRF_MOSI_PIN, NRF_CSN_PIN);  //SCLK, MISO, MOSI, SS
  pinMode(hspi->pinSS(), OUTPUT);                                     //HSPI SS

  delay(100);

  if (!radio_init(hspi)) {
    return;
  }

  // initialize the camera configuration
  camera_config_t config = camera_init();

  // initialize the camera itself
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  delay(SETUP_DELAY);
}

void loop() {
  // initialize the frame buffer with NULL
  camera_fb_t *fb = NULL;

  // send an image from the framebuffer
  send_image(fb);
  // return the buffer when finished working with it
  esp_camera_fb_return(fb);

  // put a delay before taking the new image
  delay(DELAY_BETWEEN_IMAGES);
}