import serial

s = serial.Serial('/dev/cu.usbserial-0001', 115200)
word = ""
image_data = ""
counter = 0

# Open a file in binary write mode

with open(f"./images/image{counter}.jpg", "wb") as file:
    while True:
        temp = s.read().decode('utf-8')  # Read and decode the byte to string
        if temp not in ('\n', '\r'):  # Check if the character is not \n or \r
            word += temp  # Add character to the word
        else:
            if word:  # If the word is not empty
                # print(word)
                # Check for the start condition
                if "Starting the writing to SD" in word:
                    image_data = word[26:]  # Capture the relevant image data
                    word = ""  # Reset word to continue accumulating bytes

                # Check for the end condition
                elif "Saved file" in word:
                    # Convert the accumulated string of image data into bytes
                    byte_data = bytes(int(b) for b in image_data.split())
                    file.write(byte_data)  # Write the bytes to the file
                    print("Finished writing image.")
                    break

                # Accumulate image data
                elif image_data:
                    image_data += " " + word  # Continue accumulating the image bytes

                word = ""  # Reset the word for the next line
counter+=1