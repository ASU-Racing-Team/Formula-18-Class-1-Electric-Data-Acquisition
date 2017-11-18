int sensorPin = A0;    // select the input pin for the potentiometer
int sensorValue = 0;  // variable to store the value coming from the sensor
void setup() 
{
  // put your setup code here, to run once:
  pinMode(13,OUTPUT);
}

void loop() 
{
  sensorValue = analogRead(sensorPin);
  // put your main code here, to run repeatedly:
  digitalWrite(13,HIGH);
  delay(sensorValue);
  digitalWrite(13,LOW);
  delay(sensorValue);
}
