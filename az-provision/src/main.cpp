#include <Arduino.h>

#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <cstdlib>

#include "wifi_configs.h"
#include "network.h"
#include "iot_configs.h"
#include "provision.h"
#include "hub_connect.h"
#include "sensors.h"  

void setup()
{
  char* iotHubHost; 
  char* clientId; 
  
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  establishConnection();
  initializeSensors();
  provisionDevice(iotHubHost, clientId); 
  connectIoTHub(iotHubHost,clientId);
  digitalWrite(LED_BUILTIN, LOW);
}

void loop()
{
  sendTelemetryLoop();
  delay(500);
}
