#include <Arduino.h>

#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <cstdlib>

#include <PubSubClient.h>

#include <az_result.h>
#include <az_span.h>
#include <az_iot_hub_client.h>

#include <base64.h>
#include <bearssl/bearssl.h>
#include <bearssl/bearssl_hmac.h>
#include <libb64/cdecode.h>

#include "iot_configs.h"
#include "network.h"

#define sizeofarray(a) (sizeof(a) / sizeof(a[0]))
#define ONE_HOUR_IN_SECS 3600
#define MQTT_PACKET_SIZE 1024

static const char *device_key = IOT_CONFIG_DEVICE_KEY;

static char sas_token[180];

static char base64_decoded_device_key[32];

static const int port = 8883;

static WiFiClientSecure wifi_client = initializeWiFiSecureClient();
static PubSubClient mqtt_client(wifi_client);

static az_iot_hub_client hubClient;

void receivedCallback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Received [");
  Serial.print(topic);
  Serial.print("]: ");
  for (unsigned int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
}

static uint32_t getSecondsSinceEpoch()
{
  return (uint32_t)time(NULL);
}

static int generateHubSasToken(char *sas_token, size_t size)
{
  uint8_t signature[512];
  az_span signature_span = az_span_create((uint8_t *)signature, sizeofarray(signature));
  az_span out_signature_span;

  uint32_t expiration = getSecondsSinceEpoch() + ONE_HOUR_IN_SECS;

  // Get signature
  if (az_result_failed(az_iot_hub_client_sas_get_signature(
          &hubClient, expiration, signature_span, &out_signature_span)))
  {
    Serial.println("Failed getting SAS signature");
    return 1;
  }

  // Base64-decode device key
  int base64_decoded_device_key_length = base64_decode_chars(device_key, strlen(device_key), base64_decoded_device_key);

  if (base64_decoded_device_key_length == 0)
  {
    Serial.println("Failed base64 decoding device key");
    return 1;
  }

  // SHA-256 encrypt
  br_hmac_key_context kc;
  unsigned char encrypted_signature[32];

  br_hmac_key_init(
      &kc, &br_sha256_vtable, base64_decoded_device_key, base64_decoded_device_key_length);

  br_hmac_context hmac_ctx;
  br_hmac_init(&hmac_ctx, &kc, 32);
  br_hmac_update(&hmac_ctx, az_span_ptr(out_signature_span), az_span_size(out_signature_span));
  br_hmac_out(&hmac_ctx, encrypted_signature);

  // Base64 encode encrypted signature
  String b64enc_hmacsha256_signature = base64::encode(encrypted_signature, br_hmac_size(&hmac_ctx));

  az_span b64enc_hmacsha256_signature_span = az_span_create(
      (uint8_t *)b64enc_hmacsha256_signature.c_str(), b64enc_hmacsha256_signature.length());

  // URl-encode base64 encoded encrypted signature
  int ret = az_iot_hub_client_sas_get_password(
      &hubClient,
      expiration,
      b64enc_hmacsha256_signature_span,
      AZ_SPAN_EMPTY,
      sas_token,
      size,
      NULL);
  if (az_result_failed(ret))
  {
    Serial.print("Failed getting SAS token. ");
    Serial.println(ret);
    return 1;
  }
  Serial.println("URL-encode Base64 signature Ok");
  return 0;
}

static void initializeHubClient(char *iotHubHost, char *clientId)
{
  if (az_result_failed(az_iot_hub_client_init(
          &hubClient,
          az_span_create((uint8_t *)iotHubHost, strlen(iotHubHost)),
          az_span_create((uint8_t *)clientId, strlen(clientId)),
          NULL)))
  {
    Serial.println("Failed initializing Azure IoT Hub client");
    return;
  }
  mqtt_client.setServer(iotHubHost, port);
  mqtt_client.setCallback(receivedCallback);
}

void connectIoTHub(char *iotHubHost, char *clientId)
{
  initializeHubClient(iotHubHost, clientId);

  if (generateHubSasToken(sas_token, sizeofarray(sas_token)) != 0)
  {
    Serial.println("Failed generating MQTT password");
    return;
  }

  char mqtt_username[200];
  // Get the MQTT user name used to connect to IoT Hub
  int ret = az_iot_hub_client_get_user_name(
      &hubClient, mqtt_username, sizeofarray(mqtt_username), NULL);
  if (az_result_failed(ret))
  {
    Serial.print("Failed to get MQTT username, return code");
    Serial.println(ret);
    return;
  }

  Serial.print("Client ID: ");
  Serial.println(clientId);

  Serial.print("Username: ");
  Serial.println(mqtt_username);

  Serial.println("Password: ");
  Serial.println(sas_token);

  mqtt_client.setBufferSize(MQTT_PACKET_SIZE);

  while (!mqtt_client.connected())
  {
    //time_t now = time(NULL);

    Serial.printf("MQTT connecting IoT Hub %s...", clientId);
    // Serial.print(clientId);
    // Serial.print("...");
    if (mqtt_client.connect(clientId, mqtt_username, sas_token))
    {
      Serial.println("connected to IoT Hub.");
    }
    else
    {
      Serial.print(" failed, status code =");
      Serial.print(mqtt_client.state());
      Serial.println(". Try again in 5 seconds.");
      
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  mqtt_client.subscribe(AZ_IOT_HUB_CLIENT_C2D_SUBSCRIBE_TOPIC);
}
