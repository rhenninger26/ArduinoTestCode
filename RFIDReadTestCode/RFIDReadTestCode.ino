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

            // For NTAG215, each page is 4 bytes, and there are 135 pages (0-134)
            unsigned char readPage[4];
            unsigned char allData[256];
            int totalBytesRead = 0;
            bool readFailed = false;

            // Read pages 4 to 30 (adjust as needed)
            for (int page = 4; page <= 30; page++) {
                if (rfid.read(page, readPage) == MI_OK) {
                    Serial.print("Page ");
                    Serial.print(page);
                    Serial.print(": ");
                    for (int i = 0; i < 4; i++) {
                        if (totalBytesRead < 256) {
                            allData[totalBytesRead++] = readPage[i];
                        }
                        if (readPage[i] < 0x10) Serial.print("0");
                        Serial.print(readPage[i], HEX);
                        Serial.print(" ");
                    }
                    Serial.print("  |  ");
                    for (int i = 0; i < 4; i++) {
                        // Buffer to hold the extracted URL
                        char url[200]; // Make sure this is large enough for your URLs
                        int urlIndex = 0;
                        bool urlStarted = false;

                        // Find the start of the URL ("http")
                        for (int i = 0; i < totalBytesRead - 4; i++) {
                            if (allData[i] == 'h' && allData[i+1] == 't' && allData[i+2] == 't' && allData[i+3] == 'p') {
                                // Copy from here until a non-printable or NDEF terminator (0xFE)
                                for (int j = i; j < totalBytesRead && urlIndex < sizeof(url) - 1; j++) {
                                    if (allData[j] < 32 || allData[j] > 126 || allData[j] == 0xFE) {
                                        break;
                                    }
                                    url[urlIndex++] = (char)allData[j];
                                }
                                break;
                            }
                        }
                        url[urlIndex] = '\0'; // Null-terminate the string

                        // Print the extracted URL
                        Serial.print("Extracted URL: ");
                        Serial.println(url);
                        if (readPage[i] >= 32 && readPage[i] <= 126) {
                            Serial.print((char)readPage[i]);
                        }
                        else {
                            Serial.print(".");
                        }
                    }
                    Serial.println();
                }
                else {
                    Serial.print("Failed to read page ");
                    Serial.println(page);
                    readFailed = true;
                    break;
                }
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
            // Extract and print only the URL from the NFC data
            Serial.println("Extracted URL:");
            bool urlStarted = false;
            int urlStart = -1;
            int urlEnd = -1;

            // Look for "http" or "https" in the data
            for (int i = 0; i < totalBytesRead - 4; i++) {
                if ((allData[i] == 'h' && allData[i + 1] == 't' && allData[i + 2] == 't' && allData[i + 3] == 'p')) {
                    urlStart = i;
                    urlStarted = true;
                    break;
                }
            }
            if (urlStarted) {
                // Find the end of the URL (stop at first non-printable ASCII or 0xFE NDEF terminator)
                for (int i = urlStart; i < totalBytesRead; i++) {
                    if (allData[i] < 32 || allData[i] > 126 || allData[i] == 0xFE) {
                        urlEnd = i;
                        break;
                    }
                }
                if (urlEnd == -1) urlEnd = totalBytesRead;

                // Print the URL
                for (int i = urlStart; i < urlEnd; i++) {
                    Serial.print((char)allData[i]);
                }
                Serial.println();
            }
            else {
                Serial.println("No URL found in NFC data.");
            }
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