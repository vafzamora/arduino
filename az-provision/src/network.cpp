#include <Arduino.h>
#include <time.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#include "wifi_configs.h"
#include "ca.h"

#define NTP_SERVERS "pool.ntp.org", "time.nist.gov"

const char *ssid = IOT_CONFIG_WIFI_SSID;
const char *password = IOT_CONFIG_WIFI_PASSWORD;

void connectToWiFi()
{
  Serial.begin(115200);
  Serial.println();
  Serial.print("Connecting to WIFI SSID ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.print("WiFi connected, IP address: ");
  Serial.println(WiFi.localIP());
}

void initializeTime()
{
  Serial.print("Setting time using SNTP");

  configTime(-3 * 3600, 0, NTP_SERVERS);
  time_t now = time(NULL);
  while (now < 1510592825)
  {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("done!");
}

char *getCurrentLocalTimeString()
{
  time_t now = time(NULL);
  return ctime(&now);
}

void printCurrentTime()
{
  Serial.print("Current time: ");
  Serial.print(getCurrentLocalTimeString());
}

WiFiClientSecure initializeWiFiSecureClient()
{
  Serial.print("Configure WiFi Secure: ");
  WiFiClientSecure wifi_client;
  if (!wifi_client.setCACert((const uint8_t *)ca_pem, ca_pem_len))
  {
    Serial.println("setCACert() FAILED");
  }
  return wifi_client;
}

void establishConnection()
{
  connectToWiFi();
  initializeTime();
  printCurrentTime();
}
