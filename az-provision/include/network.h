#include <WiFiClientSecure.h>
#include <PubSubClient.h>

WiFiClientSecure initializeWiFiSecureClient();

void establishConnection();

PubSubClient getMqttClient(); 
