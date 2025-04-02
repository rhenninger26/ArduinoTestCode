int potPin = A0; // Potentiometer output connected to analog pin 0
int potValue = 0; // Variable to store the input from the potentiometer

void setup() {
  Serial.begin(115200); // port to print to the serial monitor
}

void loop() 
{
  int potVal = analogRead(potPin); // read the potentiometer value at the input pin
  Serial.println(String(potVal)); // printing the potValue to the serial monitor
}
