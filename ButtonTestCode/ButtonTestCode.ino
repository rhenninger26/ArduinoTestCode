int buttonPin = 12; // the number of the push button pin
int buttonState = HIGH; // Record button state, and initial the state to high level
int lastButtonState = HIGH; // Record the button state of last detection
long lastChangeTime = 0; // Record the time point for button state change

void setup() 
{
	pinMode(buttonPin, INPUT); // set push button pin into input mode
	Serial.begin(115200);
}

void loop() 
{
	int nowButtonState = digitalRead(buttonPin); // Read current state of button pin
	// If button pin state has changed, record the time point
	if (nowButtonState != lastButtonState) 
    {
		lastChangeTime = millis();
	}
  
	// If button state changes, and stays stable for a while, then it should have skipped the bounce area
	if (millis() - lastChangeTime > 10) 
    {
		if (buttonState != nowButtonState) 
        { // Confirm button state has changed
			buttonState = nowButtonState;
			if (buttonState == LOW) 
            { // Low level indicates the button is pressed
				Serial.println("Button is Pressed!");
			}
            else 
            { // High level indicates the button is released
                Serial.println("Button is Released!");
            }
		}
	}
	lastButtonState = nowButtonState; // Save the state of last button
}
/*---- CIRCUIT NOTES-----
- 5 volts needed
- Use digital port, not digital pwm port
*/