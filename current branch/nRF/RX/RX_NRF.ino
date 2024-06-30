#include "multiple_spi.h"

#include <nRF24L01.h>
#include <RF24.h>
#include <SPI.h>
#include <SD.h>

#define BAUDRATE 115200
#define SET_DEBUG false
#define PACKET_SIZE 30
#define PIPE 0x1234567890LL

uint8_t *fb = nullptr;
uint8_t buf[PACKET_SIZE] = { 0 };
int image_size = 0;
int packet_counter = 0;
int image_counter = 0;

bool receiving_image = false;
bool receiving_credentials = false;

RF24 radio(4, 15);

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
  radio.openReadingPipe(1, PIPE);
  radio.startListening();

  // SD card setup
  init_sd();
}

void loop() {
  // if (radio.available()) {
  //   radio.read(&weight, sizeof(weight));
  // }
  if (radio.available()) {
    radio.read(buf, PACKET_SIZE);
    if (SET_DEBUG) {
      Serial.printf("Packet %d: ", packet_counter);
      for (int i = 0; i < PACKET_SIZE; ++i) {
        Serial.printf("%d ", buf[i]);
      }
      Serial.println();
    }

    if (got_image_end(buf)) {
      Serial.printf("Got end packet, before saving image, the size is %d\n", image_size);
      save_image_to_sd(fb, image_size);
      delete[] fb;
      image_size = 0;
      packet_counter = 0;
      receiving_image = false;
      receiving_credentials = false;
    } else if (receiving_image) {
      if (image_size - packet_counter * PACKET_SIZE >= PACKET_SIZE) {
        for (int i = 0; i < PACKET_SIZE; i++) {
          fb[packet_counter * PACKET_SIZE + i] = buf[i];
        }
      } else {
        int modulo_size = image_size - packet_counter * PACKET_SIZE;
        for (int i = 0; i < modulo_size; i++) {
          fb[(packet_counter)*PACKET_SIZE + i] = buf[i];
        }
      }
      packet_counter++;
      // Serial.printf("Packet counter: %d\n", packet_counter);
    } else if (receiving_credentials) {
      image_size = got_image_credentials(buf);
      Serial.printf("Got credentials, the image size is: %d\n", image_size);
      if (SET_DEBUG) { Serial.printf("Got image size of %d bytes\n", image_size); }
      fb = new uint8_t[image_size];
      if (fb == NULL) {
        Serial.println("Could not allocate memory");
        return;
      }
      receiving_credentials = false;
      receiving_image = true;
    } else if (got_image_start(buf)) {
      Serial.println("Got start");
      receiving_credentials = true;
      packet_counter = 0;
    }
    // the end
  }
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

bool got_image_start(uint8_t buffer[]) {
  for (int i = 0; i < PACKET_SIZE; i++) {
    if (buffer[i] != 0) {
      return false;
    }
  }
  return true;
}

int got_image_credentials(uint8_t buffer[]) {
  // int image_size = (buffer[0] + buffer[1]) * PACKET_SIZE + buffer[2];

  int full_packets = buffer[0] + (buffer[1] << 8);
  int remainder = buffer[2];
  int image_size = full_packets * PACKET_SIZE + remainder;

  if (SET_DEBUG) { Serial.printf("The size of the received image: %d\n", image_size); }
  return image_size;
}

bool got_image_end(uint8_t buffer[]) {
  for (int i = 0; i < PACKET_SIZE; i++) {
    if (buffer[i] != 1) {
      return false;
    }
  }
  return true;
}

void save_image_to_sd(uint8_t *fb, int size) {
  Serial.println("Starting the writing to SD");

  String path = "/picture" + String(image_counter) + ".jpg";

  fs::FS &fs = SD;
  Serial.printf("Picture file name: %s\n", path.c_str());

  File file = fs.open(path.c_str(), FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file in writing mode");
  } else {
    Serial.printf("Size of frame buffer is : %d\n", size);
    file.write(fb, size);
    uint8_t n1 = 255;
    uint8_t n2 = 217;
    file.write(n1);
    file.write(n2);
    Serial.printf("Saved file to path: %s\n", path.c_str());
  }
  file.close();
  image_counter++;
}
