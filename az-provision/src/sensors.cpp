#include <Arduino.h>

#include <Adafruit_Sensor.h>
#include <DHT.h>

#include "sensors.h"

//DHT 11 sensor configuration
#define DHTPIN 12
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

void initializeSensors(){
     dht.begin(); 
}

SensorRead getSensorData(){
  static SensorRead lastRead;
  
  float newValue = dht.readTemperature();
  if (!isnan(newValue))
  {
    lastRead.temperature = newValue;
  }

  newValue = dht.readHumidity();
  if (!isnan(newValue))
  {
    lastRead.humidity = newValue;
  }
  lastRead.light = analogRead(A0);

  return lastRead; 
}