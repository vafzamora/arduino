/*
  AnalogReadSerial

  Reads an analog input on pin 0, prints the result to the Serial Monitor.
  Graphical representation is available using Serial Plotter (Tools > Serial Plotter menu).
  Attach the center pin of a potentiometer to pin A0, and the outside pins to +5V and ground.

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/AnalogReadSerial
*/

const int MAX = 1023;
int okValue = MAX * 0.75;
int warnValue = MAX * 0.85;

// the setup routine runs once when you press reset:
void setup()
{
  //initialize led pins
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
}

// the loop routine runs over and over again forever:
void loop()
{
  // read the input on analog pin 0:
  int sensorValue = analogRead(A0);
  digitalWrite(10, sensorValue > 2);
  digitalWrite(11, sensorValue > okValue);
  digitalWrite(12, sensorValue > warnValue);

  // print out the value you read:
  Serial.println(sensorValue);
  delay(1); // delay in between reads for stability
}
