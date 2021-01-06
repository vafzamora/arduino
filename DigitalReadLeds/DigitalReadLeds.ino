/*
  DigitalReadSerial

  Reads a digital input on pin 2, prints the result to the Serial Monitor

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/DigitalReadSerial
*/

const int LAST_LED = 12;
const int FIRST_LED = 10;

// digital pin 2 has a pushbutton attached to it. Give it a name:
int pushButton = 2;

int previousButtonState = HIGH;

int currentLed = FIRST_LED;
int previousLed = currentLed;

// the setup routine runs once when you press reset:
void setup()
{
  // make the pushbutton's pin an input:
  pinMode(pushButton, INPUT_PULLUP);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
}

// the loop routine runs over and over again forever:
void loop()
{
  // read the input pin:
  int buttonState = digitalRead(pushButton);
  if (previousButtonState != buttonState && buttonState)
  {
    if (currentLed<=LAST_LED)
    {
      digitalWrite(previousLed, LOW);
      previousLed = currentLed;
      digitalWrite(currentLed++, HIGH);
    }
    else
    {
      digitalWrite(LAST_LED,LOW);
      currentLed = FIRST_LED;
    }
  }
  previousButtonState = buttonState;
  delay(5);
}
