#include "multiple_spi.h"

#include <nRF24L01.h>
#include <RF24.h>
#include <SPI.h>
#include <SD.h>

#define BAUDRATE 115200
#define SET_DEBUG false
#define PACKET_SIZE 30
#define PIPE 0x1234567890LL

uint8_t* fb = nullptr;
long image_size = 0;
long packet_counter = 0;
long failures = 0;

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
  radio.setDataRate(RF24_2MBPS);
  radio.setPALevel(RF24_PA_HIGH);
  radio.openWritingPipe(PIPE);

  // SD card setup
  init_sd();
  delay(5000);
}

void loop() {
  // Serial.println("Opening the SD card...");
  File img = SD.open("/picture0.jpg");
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
    //print image binary:
    // for (int i = 0; i < image_size; i++) {
    //   Serial.printf("%d ", fb[i]);
    // }
    // Serial.println();


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

    Serial.printf("Number of packets to send is: %d\n", image_size / PACKET_SIZE);

    // close the file:
    img.close();
  } else {
    Serial.println("No image found.");
  }
  int start_time = millis();
  if (send_image_start()) {
    Serial.println("The starting packet sent successfully");
    if (send_image_credentials(image_size)) {

      Serial.println("The credentials packet sent successfully");

      packet_counter = 0;
      bool packet_ack;
      uint8_t buf[PACKET_SIZE] = { 0 };
      for (int i = 0; i < image_size / PACKET_SIZE; i++) {
        for (int j = 0; j < PACKET_SIZE; j++) {
          buf[j] = fb[PACKET_SIZE * i + j];
          if (SET_DEBUG) {
            Serial.printf("%d ", buf[j]);
          }
        }
        if (SET_DEBUG) { Serial.println(); };

        // write the cur
        packet_ack = radio.write(&buf, sizeof(buf));
        if (packet_ack) {
          // Serial.printf("Packet %d sent successfully.\n", packet_counter);
        } else {
          failures++;
          Serial.printf("Packet %d failed.\n", packet_counter);
        }
        packet_counter++;
        delay(1);
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
  Serial.printf("Packets failed: %d\n", failures);
  failures = 0;
  Serial.printf("Time taken: %3.3f seconds \n", float((millis() - start_time) / 1000.0));
  Serial.println();
  delay(10000);
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
  // if (size / PACKET_SIZE > 255) {
  //   buf[0] = 255;
  //   buf[1] = size / PACKET_SIZE - 255;
  // } else {
  //   buf[0] = size / PACKET_SIZE;
  //   buf[1] = 0;
  // }
  // buf[2] = size % PACKET_SIZE;
  // for (int i = 3; i < PACKET_SIZE; i++) {
  //   buf[i] = 0;
  // }

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