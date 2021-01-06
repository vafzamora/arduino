#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <SFE_BMP180.h>
#include <Wire.h>

#define DHTPIN 12
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
SFE_BMP180 pressure;

float temp = 0.0;
float humidity = 0.0;

void setup()
{
  Serial.begin(115200);
  dht.begin();
  Serial.println();
  Serial.println("Will start Pressure");
  if (pressure.begin())
  {
    Serial.println("Pressure Sensor Ok.");
  }
  else{
    Serial.println("Pressure failed.");
  }
}

void loop()
{
  char status;
  double T, P;

  int ldrValue = analogRead(A0); 
  Serial.print("Light sensor: ");
  Serial.println(ldrValue);

  float newTemp = dht.readTemperature();
  if (!isnan(newTemp))
  {
    temp = newTemp;
  }

  float newHumidity = dht.readHumidity();
  if (!isnan(newHumidity))
  {
    humidity = newHumidity;
  }

  status = pressure.getTemperature(T);
  if (status != 0)
  {
    delay(status);
    status = pressure.getTemperature(T);
    if (status != 0)
    {
      status = pressure.startPressure(3);
      if (status != 0)
      {
        delay(status);
        status = pressure.getPressure(P, T);
        if (status != 0)
        {
          // Print out the measurement:
          Serial.print("absolute pressure: ");
          Serial.print(P, 2);
          Serial.print(" mb, ");
          Serial.print(P * 0.0295333727, 2);
          Serial.println(" inHg");
        }
      }
    }
  }

  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.print("; Humidity: ");
  Serial.println(humidity);
  delay(2000);
}