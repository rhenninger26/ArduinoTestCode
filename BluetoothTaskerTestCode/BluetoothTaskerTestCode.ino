#include <ArduinoBLE.h>

// BLE Service
BLEService messageService("19B10000-E8F2-537E-4F6C-D104768A1214"); // Custom BLE Service UUID

// BLE Characteristic for sending messages
BLEStringCharacteristic messageCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", // Custom characteristic UUID
    BLERead | BLENotify, 20); // Allow remote device to read and get updates (notify), message up to 20 bytes

// Variables for periodic message updates
unsigned long previousMillis = 0;
const long interval = 2000;      // Send a message every 2 seconds
int messageCount = 0;

void setup() {
    Serial.begin(9600);
    while (!Serial);

    // Initialize BLE hardware
    if (!BLE.begin()) {
        Serial.println("Starting BLE failed!");
        while (1);
    }

    // Set advertised local name and service UUID
    BLE.setLocalName("ArduinoR4");
    BLE.setAdvertisedService(messageService);

    // Add the characteristic to the service
    messageService.addCharacteristic(messageCharacteristic);

    // Add service
    BLE.addService(messageService);

    // Set initial value for the characteristic
    messageCharacteristic.writeValue("Hello Phone!");

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

        // While the central is still connected to peripheral
        while (central.connected()) {
            // Get the current time
            unsigned long currentMillis = millis();

            // Send a new message every 'interval' milliseconds
            if (currentMillis - previousMillis >= interval) {
                previousMillis = currentMillis;

                String message = "Message #" + String(messageCount++);
                messageCharacteristic.writeValue(message);
                Serial.println("Sent: " + message);
            }
        }

        Serial.print("Disconnected from central: ");
        Serial.println(central.address());
    }
}
