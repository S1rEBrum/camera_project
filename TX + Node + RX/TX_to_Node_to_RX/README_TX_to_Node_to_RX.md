### Short Explanation:

This is the total code for the 3-point system: transmitter, node and receiver. The trasmitter and receiver parts are finished and tested. 
The total code for the node is not finished - the receiver part works and saves images onto SD card. 
The receiver part does not succeed in sending the starting packet - meaning it has no connection to the RX.
Possible solutions: change writing pipe for the radio (tried - did not work on its own); change the channel (calculation is 2400MHz + channel value), 
current one for TX uses channel 5, try bigger value (~100) etc; check the Node-to-RX logic once more, maybe the radio is not configured and tries 
to listen to the TX pipe at the same time. 
