// FM Transmitter Control
// Connect FM transmitter power to digital pin 7
// Connect FM transmitter ground to Arduino ground

const int fmTransmitterPin = 7;  // FM transmitter connected to digital pin 7

void setup() {
	// Initialize serial communication for debugging
	Serial.begin(9600);
	Serial.println("FM Transmitter Test");

	// Set the FM transmitter pin as OUTPUT
	pinMode(fmTransmitterPin, OUTPUT);

	// Initially turn off the FM transmitter
	digitalWrite(fmTransmitterPin, HIGH);
	Serial.println("FM Transmitter ON");
}

void loop() {
	
}
