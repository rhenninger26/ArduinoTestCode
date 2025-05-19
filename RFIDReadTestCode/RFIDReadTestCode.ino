#include <SPI.h>
#include <RFID.h>
#include <ArduinoBLE.h>

// BLE Service
BLEService messageService("19B10000-E8F2-537E-4F6C-D104768A1214"); // Custom BLE Service UUID

// Connection notification characteristic - will be used to trigger Tasker
BLEStringCharacteristic connectionNotifyCharacteristic("19B10002-E8F2-537E-4F6C-D104768A1214", // New UUID for connection notification
    BLERead | BLENotify, 300);

//D10:pin of card reader SDA. D9:pin of card reader RST
RFID rfid(10, 9);
unsigned char status;
unsigned char str[MAX_LEN]; //MAX_LEN is 16: size of the array
unsigned char key[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }; // Default key for authentication

// Store the last read URL
char lastReadUrl[300] = "";
bool hasStoredUrl = false;

void setup()
{
    Serial.begin(9600);
    SPI.begin();
    rfid.init(); //initialization

    while (!Serial);
    // Initialize BLE hardware
    if (!BLE.begin()) {
        Serial.println("Starting BLE failed!");
        while (1);
    }

    // Set advertised local name and service UUID
    BLE.setLocalName("ArduinoR4");
    BLE.setAdvertisedService(messageService);

    // Add the notification characteristic to the service
    messageService.addCharacteristic(connectionNotifyCharacteristic);

    // Add service
    BLE.addService(messageService);

    // Start advertising
    BLE.advertise();

    Serial.println("Bluetooth device active, waiting for connections...");
}

void loop() {
    // Listen for BLE peripherals to connect
    BLEDevice central = BLE.central();

    // If a central is connected to peripheral
    if (central) {
        Serial.print("Connected to central: ");
        Serial.println(central.address());

        // Send connection notification with URL if available
        if (hasStoredUrl) {
            connectionNotifyCharacteristic.writeValue(lastReadUrl);
            Serial.println("Sent URL notification upon connection");
        }
        else {
            connectionNotifyCharacteristic.writeValue("No URL scanned yet");
        }

        static bool urlPrinted = false;

        // While connected
        while (central.connected()) {
            if (rfid.findCard(PICC_REQIDL, str) == MI_OK) {
                if (!urlPrinted) {
                    Serial.println("Find the card!");
                    ShowCardType(str);

                    if (rfid.anticoll(str) == MI_OK) {
                        Serial.print("The card's number is : ");
                        for (int i = 0; i < 4; i++) {
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
                                for (int i = 0; i < 4; i++) {
                                    if (totalBytesRead < 256) {
                                        allData[totalBytesRead++] = readPage[i];
                                    }
                                }
                            }
                            else {
                                readFailed = true;
                                break;
                            }
                        }

                        // Extract and print only the URL from the NFC data
                        char url[300];
                        int urlIndex = 0;
                        bool urlFound = false;

                        // Find the start of the URL ("http")
                        for (int i = 0; i < totalBytesRead - 4; i++) {
                            if (allData[i] == 'h' && allData[i + 1] == 't' && allData[i + 2] == 't' && allData[i + 3] == 'p') {
                                // Copy from here until a non-printable or NDEF terminator (0xFE)
                                for (int j = i; j < totalBytesRead && urlIndex < sizeof(url) - 1; j++) {
                                    if (allData[j] < 32 || allData[j] > 126 || allData[j] == 0xFE) {
                                        break;
                                    }
                                    url[urlIndex++] = (char)allData[j];
                                }
                                urlFound = true;
                                break;
                            }
                        }
                        url[urlIndex] = '\0'; // Null-terminate the string

                        if (urlFound) {
                            Serial.print("Extracted URL: ");
                            Serial.println(url);

                            // Store the URL for future connections
                            strncpy(lastReadUrl, url, sizeof(lastReadUrl) - 1);
                            lastReadUrl[sizeof(lastReadUrl) - 1] = '\0'; // Ensure null-termination
                            hasStoredUrl = true;

                            // // Send the URL via Bluetooth
                            // messageCharacteristic.writeValue(url);
                            // Also update the connection notification characteristic
                            connectionNotifyCharacteristic.writeValue(url);
                            Serial.println("Sent URL via Bluetooth");
                        }
                        else {
                            Serial.println("No URL found in NFC data.");
                        }
                        urlPrinted = true;
                    }
                }
            }
            else {
                urlPrinted = false; // Reset flag when no card is present
            }
            rfid.halt(); // Command the card to enter sleeping state
        }

        Serial.print("Disconnected from central: ");
        Serial.println(central.address());
    }
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