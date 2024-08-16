The code for communication between 2 ESP32 modules using nRF24L01+ transciever modules. 
The transmitter reads the image from SD card, parces it into packets and sends via the protocol.
The receiver listens to the radio pipe on the specific channel, when gets indication of getting image, saves it to the buffer and then onto the SD card. 
The multiple_spi.h header files are needed in both in order to initialize VSPI (virtual) and HSPI (hardware) serial buses. 
