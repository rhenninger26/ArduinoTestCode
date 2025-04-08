int in1Pin = 10; // Define L293D channel 1 pin
int in2Pin = 9; // Define L293D channel 2 pin
int enable1Pin = 11; // Define L293D enable 1 pin
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
  int potenVal = analogRead(A0); // Convert the voltage of rotary potentiometer into digital
  Serial.println(String(potenVal));

  // Compare the number with value 512, if more than 512, clockwise rotates, otherwise, counterclockwise rotates
  rotationSpeed = potenVal - 512;

  if (potenVal > 512)
  {
    rotationDir = true;
  }
  else
  {
    rotationDir = false;
  }

  // Calculate the motor speed, the far number of deviation from the middle value 512, the faster the control speed will be
  rotationSpeed = 150;

  // Control the steering and speed of the motor
  driveMotor(rotationDir,map(rotationSpeed, 0, 512, 0, 255));
}

void driveMotor(boolean dir, int spd) {
  // Control motor rotation direction
  if (dir) 
  {
    digitalWrite(in1Pin, HIGH);
    digitalWrite(in2Pin, LOW);
  }
  else 
  {
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, HIGH);
  }
  // Control motor rotation speed
  analogWrite(enable1Pin, constrain(60, 0, 255));
}

/*---- NOTES FOR CIRCUIT HOOKUP-----

- Potentiometer data wire (blue) goes to analog port (A0 - A5)
- 5 volts
- L293D wires (green, white, white) should go to PWM ports, (I used ~11(left green), ~10(middle white), and ~9(right white))
*/