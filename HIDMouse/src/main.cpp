#include <Arduino.h>
#include <BleMouse.h>
#include <BleKeyboard.h> 

#define UP_BUTTON 15
#define DOWN_BUTTON 14
#define FULL_SCREEN_BUTTON 13

BleMouse bleMouse;
BleKeyboard bleKeyboard; 

void setup()
{
    Serial.begin(115200);
    pinMode(UP_BUTTON, INPUT);
    pinMode(DOWN_BUTTON, INPUT);
    pinMode(FULL_SCREEN_BUTTON, INPUT);

    Serial.println("Starting BLE work!");
    //bleMouse.begin();
    bleKeyboard.begin(); 
}

void loop()
{
    if (bleMouse.isConnected())
    {
        if (digitalRead(UP_BUTTON) == HIGH)
        {
            bleMouse.move(0, 0, 1);
            // Serial.print("UP: ");
            // Serial.println(digitalRead(UP_BUTTON));
        }
        if (digitalRead(DOWN_BUTTON)==HIGH)
        {
            bleMouse.move(0, 0, -1);
            // Serial.print("DOWN: ");
            // Serial.println(digitalRead(DOWN_BUTTON));
        }
    }
    if(bleKeyboard.isConnected()){
        if (digitalRead(FULL_SCREEN_BUTTON)==HIGH)
        {
            bleKeyboard.write(KEY_F11);
            delay(100);
            Serial.print("FULL_SCREEN: ");
            Serial.println(digitalRead(FULL_SCREEN_BUTTON));
        }
    }
    delay(50);
}