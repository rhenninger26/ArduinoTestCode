int in1Pin = 5; // Define L293D channel 1 pin
int in2Pin = 3; // Define L293D channel 2 pin
int enable1Pin = 6; // Define L293D enable 1 pin
boolean rotationDir; // Define a variable to save the motor's rotation direction, true and false are represented by positive rotation and reverse rotation.
int rotationSpeed; // Define a variable to save the motor rotation speed

void setup() 
{
  // Initialize the pin into an output mode:
  pinMode(in1Pin, OUTPUT);
  pinMode(in2Pin, OUTPUT);
  pinMode(enable1Pin, OUTPUT);

  // Serial monitor port
  Serial.begin(115200);
}
void loop() {
    int potenVal = analogRead(A3); // Read potentiometer value
    Serial.println(String(potenVal));

    // Determine rotation direction
    if (potenVal > 512) {
        rotationDir = true; // Clockwise
        rotationSpeed = potenVal - 512; // Speed based on deviation from center
    }
    else {
        rotationDir = false; // Counterclockwise
        rotationSpeed = 512 - potenVal; // Speed based on deviation from center
    }

    // Map the speed to a usable PWM range
    int pwmSpeed = map(rotationSpeed, 0, 512, 0, 255);

    // Drive the motor
    driveMotor(rotationDir, pwmSpeed);
}

void driveMotor(boolean dir, int spd) {
    // Control motor rotation direction
    if (dir) {
        digitalWrite(in1Pin, HIGH);
        digitalWrite(in2Pin, LOW);
    }
    else {
        digitalWrite(in1Pin, LOW);
        digitalWrite(in2Pin, HIGH);
    }

    // Control motor rotation speed
    analogWrite(enable1Pin, constrain(spd, 0, 255)); // Use the mapped speed
}

/*---- NOTES FOR CIRCUIT HOOKUP-----

- Potentiometer data wire (blue) goes to analog port (A0 - A5)
- 5 volts
- L293D wires (green, white, white) should go to PWM ports, (I used ~11(left green), ~10(middle white), and ~9(right white))
*/