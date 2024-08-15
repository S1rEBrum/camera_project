import serial
import time

s = serial.Serial('/dev/cu.usbserial-0001', 115200, timeout=None)

global counter
counter = 0
word = ""
image_data = ""

while True:
    if s.inWaiting() > 0:

        with open(f"./images/image{counter}.jpg", "wb") as file:
            start_time = time.time()  # Record the start time for processing
            counter += 1
            while True:
                # Read and decode the byte to string
                temp = s.read().decode('utf-8', errors='ignore')
                if temp not in ('\n', '\r'):  # Check if the character is not \n or \r
                    word += temp  # Add character to the word
                else:
                    if word:  # If the word is not empty
                        # print(word)
                        # Check for the start condition
                        if "Starting the writing to SD" in word:
                            # Capture the relevant image data
                            image_data = word[26:]
                            word = ""  # Reset word to continue accumulating bytes

                        # Check for the end condition
                        elif "Saved file" in word:
                            # Convert the accumulated string of image data into bytes
                            try:
                                byte_data = bytes(int(b)
                                                  for b in image_data.split())
                                # Write the bytes to the file
                                file.write(byte_data)
                                print(f"Finished writing image{counter-1}.jpg")

                                # Calculate and print the elapsed time
                                elapsed_time = time.time() - start_time
                                print(f"Time taken for image{counter-1}.jpg: {elapsed_time:.2f} seconds")
                            except ValueError as e:
                                print(f"Error converting image data: {e}")

                            word = ""
                            break

                        # Accumulate image data
                        elif image_data:
                            image_data += " " + word  # Continue accumulating the image bytes

                        word = ""  # Reset the word for the next line

    time.sleep(1)
