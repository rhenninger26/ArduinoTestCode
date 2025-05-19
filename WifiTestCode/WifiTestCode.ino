#include <WiFiS3.h>
#include "arduino_secrets.h"
#include <SPI.h>
#include <RFID.h>

// D10:pin of card reader SDA. D9:pin of card reader RST
RFID rfid(10, 9);
#ifndef MAX_LEN
#define MAX_LEN 16
#endif
unsigned char status;
unsigned char str[MAX_LEN];
unsigned char key[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

char lastReadUrl[300] = "";
//String storedUrl = "";
//String currentURL = "";
bool urlPrinted = false;

// Pushover API settings
const char* pushoverHost = "api.pushover.net";
const int   pushoverPort = 443;
const char* pushoverToken = PUSHOVER_TOKEN;
const char* pushoverUser = PUSHOVER_ID;

WiFiSSLClient client;
int wifiStatus = WL_IDLE_STATUS;

void setup() {
	Serial.begin(9600);
	SPI.begin();
	rfid.init();

	Serial.println("Connecting to wifi...");

	if (WiFi.status() == WL_NO_MODULE) {
		Serial.println("Communication with WiFi module failed!");
		while (true);
	}

	String fv = WiFi.firmwareVersion();
	if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
		Serial.println("Please upgrade the firmware");
	}

	while (wifiStatus != WL_CONNECTED) {
		Serial.print("Attempting to connect to WPA SSID: ");
		Serial.println(SECRET_SSID);
		wifiStatus = WiFi.begin(SECRET_SSID, SECRET_PASS);
		delay(10000);
	}
	Serial.println("You're connected to the network");
}

void loop() {
	// Example: send a pushover notification every 60 seconds
	// String testUrl = "https://example.com";
	// sendPushoverNotification(testUrl);
	// delay(60000);

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
					//hasStoredUrl = true;

					// // Send the URL
					sendPushoverNotification(url);
					Serial.println("Sent URL via Pushover");
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

void sendPushoverNotification(const String& targetURL) {
	String postData = "token=" + String(pushoverToken) +
		"&user=" + String(pushoverUser) +
		"&message=" + targetURL +
		"&url=" + targetURL +
		"&url_title=" + "Launch+Website";

	Serial.print("Connecting to Pushover... ");
	if (!client.connect(pushoverHost, pushoverPort)) {
		Serial.println("Connection failed.");
		return;
	}
	Serial.println("Connected!");

	String request = String("POST ") + "/1/messages.json" + " HTTP/1.1\r\n" +
		"Host: " + pushoverHost + "\r\n" +
		"User-Agent: Arduino\r\n" +
		"Content-Type: application/x-www-form-urlencoded\r\n" +
		"Content-Length: " + postData.length() + "\r\n" +
		"Connection: close\r\n\r\n" +
		postData;

	client.print(request);
	Serial.println("Request sent:");
	Serial.println(request);

	Serial.println("Server response:");
	while (client.connected() || client.available()) {
		if (client.available()) {
			String line = client.readStringUntil('\n');
			Serial.println(line);
		}
	}
	client.stop();
	Serial.println("Connection closed.");
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