#include <Arduino.h>

#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <cstdlib>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <Adafruit_Sensor.h>
#include <DHT.h>

#include "wifi_configs.h"
#include "network.h"
#include "iot_configs.h"
#include "provision.h"
#include "hub_connect.h"

//DHT 11 sensor configuration
#define DHTPIN 12
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

struct DhtSensorRead{
  float temperature;
  float humidity;
};

// static unsigned long next_telemetry_send_time_ms = 0;
// static char telemetry_topic[128];
// static uint8_t telemetry_payload[100];
// static uint32_t telemetry_send_count = 0;

void setup()
{
  char* iotHubHost; 
  char* clientId; 
  
  Serial.begin(115200);
  //dht.begin();
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  establishConnection();
  provisionDevice(iotHubHost, clientId); 
  connectIoTHub(iotHubHost,clientId);
  digitalWrite(LED_BUILTIN, LOW);
}

void loop()
{
  // put your main code here, to run repeatedly:
}