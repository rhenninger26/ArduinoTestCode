#include <SPI.h>
#include <RFID.h>
//D10:pin of card reader SDA. D9:pin of card reader RST
RFID rfid(10, 9);
unsigned char status;
unsigned char str[MAX_LEN]; //MAX_LEN is 16: size of the array
unsigned char key[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }; // Default key for authentication

void setup()
{
  Serial.begin(9600);
  SPI.begin();
  rfid.init(); //initialization
  //Serial.println("Please put the card to the induction area...");
  Serial.println("Place the card on the reader to test writing...");
}
void loop()
{
    // ---------- WRITE TEST CODE ------------
    // Search for a card
    // if (rfid.findCard(PICC_REQIDL, str) == MI_OK) {
    //     Serial.println("Card detected!");

    //     // Anti-collision detection to get the card's serial number
    //     if (rfid.anticoll(str) == MI_OK) {
    //         Serial.print("Card UID: ");
    //         for (int i = 0; i < 4; i++) {
    //             Serial.print(0x0F & (str[i] >> 4), HEX);
    //             Serial.print(0x0F & str[i], HEX);
    //         }
    //         Serial.println("");

    //         // Authenticate block 4 (or any writable block)
    //         if (rfid.auth(PICC_AUTHENT1A, 16, key, str) == MI_OK) {
    //             Serial.println("Authentication successful!");

    //             // Data to write (16 bytes max for a block)
    //             unsigned char dataBlock[16] = "SpotifyURI";

    //             // Write data to block 4
    //             if (rfid.write(4, dataBlock) == MI_OK) {
    //                 Serial.println("Data written successfully!");

    //                 // Read back the data to verify
    //                 unsigned char readBlock[16];
    //                 if (rfid.read(4, readBlock) == MI_OK) {
    //                     Serial.print("Data read from block 4: ");
    //                     for (int i = 0; i < 16; i++) {
    //                         Serial.print((char)readBlock[i]);
    //                     }
    //                     Serial.println("");
    //                 }
    //                 else {
    //                     Serial.println("Failed to read back data.");
    //                 }
    //             }
    //             else {
    //                 Serial.println("Failed to write data.");
    //             }

    //             //rfid.stopCrypto1(); // Stop encryption
    //         }
    //         else {
    //             Serial.println("Authentication failed.");
    //         }
    //     }
    //     rfid.halt(); // Command the card to enter sleeping state
    // }

    //---------- READ TEST CODE ------------
    if (rfid.findCard(PICC_REQIDL, str) == MI_OK)
    {
        Serial.println("Find the card!");
        // Show card type
        ShowCardType(str);

        if (rfid.anticoll(str) == MI_OK) {
            Serial.print("The card's number is : ");
            for (int i = 0; i < 4; i++)
            {
                Serial.print(0x0F & (str[i] >> 4), HEX);
                Serial.print(0x0F & str[i], HEX);
            }
            Serial.println("");

            // For NTAG215 - attempt to read all data blocks
            unsigned char readBlock[16];
            Serial.println("Reading all available data blocks...");

            // Create buffers to store all the data
            unsigned char allData[256]; // More than enough for 84 bytes
            int totalBytesRead = 0;
            bool readFailed = false;

            // Try reading more blocks - NTAG215 has 135 pages of 4 bytes each
            // We'll try reading blocks 4 through 40 (should cover your 84 bytes)
            for (int block = 4; block <= 30; block++) {
                if (rfid.read(block, readBlock) == MI_OK) {
                    Serial.print("Block ");
                    Serial.print(block);
                    Serial.print(": ");

                    // Display as HEX and ASCII
                    for (int i = 0; i < 16; i++) {
                        // Store the data
                        if (totalBytesRead < 256) {
                            allData[totalBytesRead++] = readBlock[i];
                        }

                        // Print HEX
                        if (readBlock[i] < 0x10) Serial.print("0");
                        Serial.print(readBlock[i], HEX);
                        Serial.print(" ");
                    }

                    Serial.print("  |  ");

                    // ASCII representation
                    for (int i = 0; i < 16; i++) {
                        if (readBlock[i] >= 32 && readBlock[i] <= 126) {
                            Serial.print((char)readBlock[i]);
                        }
                        else {
                            Serial.print(".");
                        }
                    }
                    Serial.println();
                }
                else {
                    Serial.print("Failed to read block ");
                    Serial.println(block);
                    readFailed = true;
                    break; // Stop if reading fails
                }

                // Check if we've read enough data (at least 84 bytes)
                if (totalBytesRead >= 84 && readFailed) {
                    break;
                }
            }

            // Display summary of all data
            Serial.println("\n---------- COMPLETE DATA SUMMARY ----------");
            Serial.print("Total bytes read: ");
            Serial.println(totalBytesRead);

            // Show all data in one continuous string (ASCII)
            Serial.println("All data (ASCII):");
            for (int i = 0; i < totalBytesRead; i++) {
                if (allData[i] >= 32 && allData[i] <= 126) {
                    Serial.print((char)allData[i]);
                }
                else {
                    Serial.print(".");
                }
            }
            Serial.println("\n");

            // Show all data in hexadecimal
            Serial.println("All data (HEX):");
            for (int i = 0; i < totalBytesRead; i++) {
                if (allData[i] < 0x10) Serial.print("0");
                Serial.print(allData[i], HEX);
                Serial.print(" ");
                if ((i + 1) % 16 == 0) Serial.println();
            }
            Serial.println("\n");

            rfid.selectTag(str);
        }
    }
    rfid.halt(); // command the card to enter sleeping state
}

void ShowCardType(unsigned char* type)
{
    Serial.print("Card type: ");
    Serial.print("Raw type bytes: ");
    Serial.print(type[0], HEX);
    Serial.print(" ");
    Serial.println(type[1], HEX);

    if (type[0] == 0x04 && type[1] == 0x00)
        Serial.println("MFOne-S50");
    else if (type[0] == 0x02 && type[1] == 0x00)
        Serial.println("MFOne-S70");
    else if (type[0] == 0x44 && type[1] == 0x00)
        Serial.println("MF-UltraLight");
    else if (type[0] == 0x08 && type[1] == 0x00)
        Serial.println("MF-Pro");
    else if (type[0] == 0x44 && type[1] == 0x03)
        Serial.println("MF Desire");
    else if (type[0] == 0x04 && type[1] == 0x03)
        Serial.println("NTAG215");
    else
        Serial.println("Unknown");
}

/* ------ EXTRA INFO -------
- White card reads from <= 1.5"
- Blue chip reads from <= 1.5"
- 3.3 volts
- 2 digital pins 
- 3 pwm pins
*/