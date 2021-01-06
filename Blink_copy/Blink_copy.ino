/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_1 is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://www.arduino.cc/en/Main/Products

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/Blink
*/
  const int LED_1 = 12;
  const int LED_2 = 11;
  const int LED_3 = 10;
  const int LAST_PIN = 12;
  const int FIRST_PIN = 10;
  
  int counter = 10; 
// the setup function runs once when you press reset or power the board
// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_1 as an output.
  for(int i = FIRST_PIN; i<=LAST_PIN; ++i){
    pinMode(i, OUTPUT);
  }
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(counter, HIGH);
  delay(1000);                       // wait for a second
  digitalWrite(counter++, LOW);
  if(counter>LAST_PIN){
    counter = FIRST_PIN; 
  }
}
