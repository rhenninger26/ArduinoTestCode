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
    if (rfid.findCard(PICC_REQIDL, str) == MI_OK) {
        Serial.println("Card detected!");

        // Anti-collision detection to get the card's serial number
        if (rfid.anticoll(str) == MI_OK) {
            Serial.print("Card UID: ");
            for (int i = 0; i < 4; i++) {
                Serial.print(0x0F & (str[i] >> 4), HEX);
                Serial.print(0x0F & str[i], HEX);
            }
            Serial.println("");

            // Authenticate block 4 (or any writable block)
            if (rfid.auth(PICC_AUTHENT1A, 16, key, str) == MI_OK) {
                Serial.println("Authentication successful!");

                // Data to write (16 bytes max for a block)
                unsigned char dataBlock[16] = "SpotifyURI";

                // Write data to block 4
                if (rfid.write(4, dataBlock) == MI_OK) {
                    Serial.println("Data written successfully!");

                    // Read back the data to verify
                    unsigned char readBlock[16];
                    if (rfid.read(4, readBlock) == MI_OK) {
                        Serial.print("Data read from block 4: ");
                        for (int i = 0; i < 16; i++) {
                            Serial.print((char)readBlock[i]);
                        }
                        Serial.println("");
                    }
                    else {
                        Serial.println("Failed to read back data.");
                    }
                }
                else {
                    Serial.println("Failed to write data.");
                }

                //rfid.stopCrypto1(); // Stop encryption
            }
            else {
                Serial.println("Authentication failed.");
            }
        }
        rfid.halt(); // Command the card to enter sleeping state
    }

    // ---------- READ TEST CODE ------------
    ////Search card, return card types
    //if (rfid.findCard(PICC_REQIDL, str) == MI_OK) 
    //{
    //  Serial.println("Find the card!");
    //  // Show card type
    //  ShowCardType(str);
    //  //Anti-collision detection, read card serial number
    //  if (rfid.anticoll(str) == MI_OK) {
    //    Serial.print("The card's number is : ");
    //  //Display card serial number
    //    for (int i = 0; i < 4; i++) 
    //    {
    //      Serial.print(0x0F & (str[i] >> 4), HEX);
    //      Serial.print(0x0F & str[i], HEX);
    //    }
    //    Serial.println("");
    //}
    //  //card selection (lock card to prevent redundant read, removing the line will make the sketch read cards continually)
    //  rfid.selectTag(str);
    //}
    //rfid.halt(); // command the card to enter sleeping state
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