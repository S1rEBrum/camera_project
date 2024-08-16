Code snippets for the full system: 
* Node_to_RX: part of the code that tests getting the image from SD card on node and getting it to SD card on receiver.
* TX_to_Node: part of the code that takes the photo (on ESP32-CAM module) and sends it to the Node, there the image is saved on SD card.
* TX_to_Node_to_RX: code for the full 3-point system: image is taken with ESP32-CAM Transmitter, sent to Node, there saved to SD and transmitted to the Receiver and saved to the SD again.

Important: all the communication is done via nRF protocol. 
