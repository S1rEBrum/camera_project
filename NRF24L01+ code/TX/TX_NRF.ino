#include "multiple_spi.h"

#include <nRF24L01.h>
#include <RF24.h>
#include <SPI.h>
#include <SD.h>

#define BAUDRATE 115200
#define SET_DEBUG true
#define PACKET_SIZE 30
#define PIPE 0x1234567890LL

uint8_t* fb = nullptr;
long image_size = 0;
long packet_counter = 0;

RF24 radio(4, 15);  // CE and CS pin

void setup() {

  //initialize two instances of the SPIClass attached to VSPI and HSPI respectively
  vspi = new SPIClass(VSPI);
  hspi = new SPIClass(HSPI);

  vspi->begin();
  hspi->begin();

  vspi->begin(VSPI_SCLK, VSPI_MISO, VSPI_MOSI, VSPI_SS);  //SCLK, MISO, MOSI, SS
  hspi->begin(HSPI_SCLK, HSPI_MISO, HSPI_MOSI, HSPI_SS);  //SCLK, MISO, MOSI, SS

  pinMode(vspi->pinSS(), OUTPUT);  //VSPI SS
  pinMode(hspi->pinSS(), OUTPUT);  //HSPI SS

  delay(1000);
  Serial.begin(BAUDRATE);

  // radio setup
  if (!radio.begin(hspi)) {
    Serial.println("Could not initialize radio");
    return;
  } else {
    Serial.println("Radio initialized successfully");
  }
  radio.setChannel(5);
  radio.setDataRate(RF24_1MBPS);
  radio.setPALevel(RF24_PA_HIGH);
  radio.openWritingPipe(PIPE);

  // SD card setup
  init_sd();
}

void loop() {
  // Serial.println("Opening the SD card...");
  File img = SD.open("/image0.jpg");
  if (img) {
    image_size = img.size();
    Serial.printf("Image size is %d bytes\n", image_size);
    fb = new uint8_t[image_size * sizeof(uint8_t)];
    if (fb == NULL) {
      Serial.println("Could not allocate the memory.");
      delete[] fb;
      return;
    }
    // read until there's nothing else in it:
    while (img.available()) {
      img.read(fb, image_size);
    }

    // if (SET_DEBUG) {
    //   int cnt = 0;
    //   for (int i = 0; i < image_size; i++) {
    //     ++cnt;
    //     Serial.printf("%d ", fb[i]);
    //     if (cnt % PACKET_SIZE == 0) {
    //       Serial.println();
    //     }
    //   }
    //   Serial.println();
    //   Serial.println();
    // }

    Serial.printf("Number of packets to send is: %d\n", image_size/PACKET_SIZE);

    // close the file:
    img.close();
  } else {
    Serial.println("No image found.");
  }
  if (send_image_start()) {
    Serial.println("The starting packet sent successfully");
    if (send_image_credentials(image_size)) {
      Serial.println("The credentials packet sent successfully");
      packet_counter = 0;
      bool packet_ack;
      uint8_t buf[PACKET_SIZE] = { 0 };
      for (int i = 0; i < image_size / PACKET_SIZE; i++) {
        for (int j = 0; j < PACKET_SIZE; j++) {
          buf[j] = fb[packet_counter * i + j];
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

  delay(5000);
}


// ROUTINS
void init_sd() {
  Serial.println("Starting SD Card...");
  // vspi->begin(VSPI_SCLK, VSPI_MISO, VSPI_MOSI, VSPI_SS);
  if (!SD.begin(VSPI_SS, *vspi)) {
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

bool send_image_start() {
  uint8_t buf[PACKET_SIZE] = { 0 };
  bool sent = radio.write(buf, sizeof(buf));
  if (sent) { return true; };
  return false;
}

bool send_image_credentials(int size) {
  uint8_t buf[PACKET_SIZE];
  if (size / PACKET_SIZE > 255) {
    buf[0] = 255;
    buf[1] = size / PACKET_SIZE - 255;
  } else {
    buf[0] = size / PACKET_SIZE;
    buf[1] = 0;
  }
  buf[2] = size % PACKET_SIZE;
  for (int i = 3; i < PACKET_SIZE; i++) {
    buf[i] = 0;
  }
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