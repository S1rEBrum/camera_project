The code contains two parts: transmitter (TX) and receiver (RX). 

The transmitter part is uploaded to the ESP32-CAM module. It takes a picture, saves it into the buffer. The buffer is then sliced to packets of arbitrary size that are then sent. There are special first, last and credentials packets that are sent in order to give the receiver sign that the image is started. After sending, the buffer is freed and after delay the code goes over the cycle again.

The receiver part can be uploaded to the regular ESP32 module. It contains couple of functions that recognize special packets (start, credentials and end) and the main function that after getting start flag writes the image into buffer. The buffer then is saved to the SD card.

Current issues: 
- the photos have part of last row of pixels greyed out or scrambled - may be packet loss
- after taking some amount of photos - they begin to scramble some parts - possible solution - clear the buffer after 4-5 photos
- the feedback "handshake" system is not working - sends "OK" signal at all times
