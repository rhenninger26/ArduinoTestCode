int potPin = A3; // Potentiometer output connected to analog pin 0
void setup() {
  Serial.begin(9600); // port to print to the serial monitor
}

void loop() 
{
  int avgPotVal = 0;

  for(int i; i < 50; i++)
  {
    int potVal = analogRead(potPin);
    avgPotVal = avgPotVal + potVal;

  }  
  avgPotVal = avgPotVal/50;
  Serial.println(String(avgPotVal)); // printing the potValue to the serial monitor

  delay(100);
}
