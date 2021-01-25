#include <Arduino.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <cstdlib>
#include <PubSubClient.h>

#include <base64.h>
#include <bearssl/bearssl.h>
#include <bearssl/bearssl_hmac.h>
#include <libb64/cdecode.h>

#include <az_result.h>
#include <az_span.h>
#include <az_iot_provisioning_client.h>

#include "iot_configs.h"
#include "network.h"

#define sizeofarray(a) (sizeof(a) / sizeof(a[0]))
#define ONE_HOUR_IN_SECS 3600
#define MQTT_PACKET_SIZE 1024

static char _iotHubHost[128];
static char _clientId[128];

static const char *globalEndpoint = IOT_CONFIG_GLOBAL_ENDPOINT;
static const char *idscope = IOT_CONFIG_IDSCOPE;
static const char *device_id = IOT_CONFIG_DEVICE_ID;
static const char *device_key = IOT_CONFIG_DEVICE_KEY;
static const char *provisioningServer = IOT_CONFIG_PROVISIONING_SERVER;
static const int port = 8883;

static char base64_decoded_device_key[32];
static char sas_token[180];

static az_iot_provisioning_client provClient;

static WiFiClientSecure wifi_client = initializeWiFiSecureClient();
static PubSubClient mqtt_client(wifi_client);

static void initializeProvisioningClient()
{
  if (az_result_failed(az_iot_provisioning_client_init(
          &provClient,
          az_span_create((uint8_t *)globalEndpoint, strlen(globalEndpoint)),
          az_span_create((uint8_t *)idscope, strlen(idscope)),
          az_span_create((uint8_t *)device_id, strlen(device_id)),
          NULL)))
  {
    Serial.println("Failed initializing Azure IoT DPS client");
    return;
  }
}

static uint32_t getSecondsSinceEpoch()
{
  return (uint32_t)time(NULL);
}

static int generateSasToken(char *sas_token, size_t size)
{
  uint8_t signature[512];
  az_span signature_span = az_span_create((uint8_t *)signature, sizeofarray(signature));
  az_span out_signature_span;

  uint32_t expiration = getSecondsSinceEpoch() + ONE_HOUR_IN_SECS;

  // Get signature
  int ret = az_iot_provisioning_client_sas_get_signature(
          &provClient, expiration, signature_span, &out_signature_span);
  if (az_result_failed(ret))
  {
    Serial.printf("Failed getting SAS signature (%u)",ret);
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
  unsigned char encrypted_signature[32];
  br_hmac_key_context kc;
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
  ret = az_iot_provisioning_client_sas_get_password(
          &provClient,
          b64enc_hmacsha256_signature_span,
          expiration,
          AZ_SPAN_EMPTY,
          sas_token,
          size,
          NULL);
  if (az_result_failed(ret))
  {
    Serial.printf("Failed getting SAS token (%i)", ret);
    return 1;
  }

  return 0;
}

static bool registerCallbackReceived = false;

void parse_device_registration_status_message(
    char *topic,
    int topic_len,
    byte *payload,
    unsigned int length,
    az_iot_provisioning_client_register_response *out_register_response)
{
  az_result rc;
  az_span const topic_span = az_span_create((uint8_t *)topic, topic_len);
  az_span const message_span = az_span_create((uint8_t *)payload, length);

  // Parse message and retrieve register_response info.
  rc = az_iot_provisioning_client_parse_received_topic_and_payload(
      &provClient, topic_span, message_span, out_register_response);
  if (az_result_failed(rc))
  {
    Serial.printf("Message from unknown topic: az_result return code 0x%08x.", rc);
    return;
  }
  Serial.println("Client received a valid topic response:");
}

static void send_operation_query_message(
    az_iot_provisioning_client_register_response const *register_response)
{
  // Get the Query Status topic to publish the query status request.
  char query_topic_buffer[256];
  if (az_result_failed(az_iot_provisioning_client_query_status_get_publish_topic(
          &provClient,
          register_response->operation_id,
          query_topic_buffer,
          sizeof(query_topic_buffer),
          NULL)))
  {
    Serial.println(
        "Unable to get query status publish topic: az_result return code 0x%08x.");
    return;
  }

  // IMPORTANT: Wait the recommended retry-after number of seconds before query.
  Serial.printf("Querying after %u seconds...", register_response->retry_after_seconds);
  delay(register_response->retry_after_seconds * 1000);

  // Publish the query status request.
  if (!mqtt_client.publish(query_topic_buffer, ""))
  {
    Serial.println("Failed to publish query status request: MQTTClient return code %d.");
    return;
  }
}

static void handle_device_registration_status_message(
    az_iot_provisioning_client_register_response const *register_response,
    bool *ref_is_operation_complete)
{
  *ref_is_operation_complete = az_iot_provisioning_client_operation_complete(register_response->operation_status);

  // If operation is not complete, send query. On return, will loop to receive new operation
  // message.
  if (!*ref_is_operation_complete)
  {
    Serial.println("Operation is still pending.");

    send_operation_query_message(register_response);
    Serial.println("Client sent operation query message.");
  }
  else // Operation is complete.
  {
    if (register_response->operation_status == AZ_IOT_PROVISIONING_STATUS_ASSIGNED) // Successful assignment
    {
      Serial.println("Device provisioned:");
      az_span temp_span = az_span_create((uint8_t *)_iotHubHost, sizeofarray(_iotHubHost));
      Serial.print("Hub Hostname: ");
      temp_span = az_span_copy(temp_span, register_response->registration_state.assigned_hub_hostname);
      temp_span = az_span_copy_u8(temp_span, '\0');
      Serial.print(_iotHubHost);
    }
    else // Unsuccessful assignment (unassigned, failed or disabled states)
    {
      Serial.println("Device provisioning failed:");
      return;
    }
  }
}

void receiveRegisterCallback(char *topic, byte *payload, unsigned int length)
{
  Serial.println("Callback received: ");
  Serial.print("Topic: ");
  Serial.println(topic);
  Serial.print("Size of topic: ");
  Serial.println(sizeof(topic));
  az_iot_provisioning_client_register_response register_response;
  parse_device_registration_status_message(topic, strlen(topic), payload, length, &register_response);

  handle_device_registration_status_message(&register_response, &registerCallbackReceived);
}

static int connectToAzureProvisioningService()
{

  if (az_result_failed(az_iot_provisioning_client_get_client_id(
          &provClient,
          _clientId,
          sizeof(_clientId),
          NULL)))
  {
    Serial.println("Failed to get MQTT client id: az_result return code 0x%08x.");
    return 1;
  }

  char mqtt_username[128];
  // Get the MQTT user name used to connect to IoT Hub
  if (az_result_failed(az_iot_provisioning_client_get_user_name(
          &provClient, mqtt_username, sizeofarray(mqtt_username), NULL)))
  {
    printf("Failed to get MQTT clientId, return code\n");
    return 1;
  }

  Serial.print("Client ID: ");
  Serial.println(_clientId);

  Serial.print("Username: ");
  Serial.println(mqtt_username);

  Serial.print("Password: ");
  Serial.println(sas_token);
  Serial.print("Endpoint: ");
  Serial.println(provisioningServer);

  mqtt_client.setServer(provisioningServer, port);
  mqtt_client.setBufferSize(MQTT_PACKET_SIZE);
  mqtt_client.setCallback(receiveRegisterCallback);

  while (!mqtt_client.connected())
  {
    //time_t now = time(NULL);

    Serial.print("MQTT connecting ... ");
    if (mqtt_client.connect(_clientId, mqtt_username, sas_token))
    {
      Serial.println("connected.");
    }
    else
    {
      Serial.print("failed, status code =");
      Serial.print(mqtt_client.state());
      Serial.println(". Try again in 5 seconds.");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  char register_topic_buffer[128];
  if (az_result_failed(az_iot_provisioning_client_register_get_publish_topic(
          &provClient, register_topic_buffer, sizeof(register_topic_buffer),
          NULL)))
  {
    Serial.println("Failed to get register topic");
  }

  if (!mqtt_client.subscribe(AZ_IOT_PROVISIONING_CLIENT_REGISTER_SUBSCRIBE_TOPIC, 1))
  {
    Serial.println("Error subscribing topic.");
  }

  if (mqtt_client.publish(register_topic_buffer, "", false))
  {
    Serial.println("Message published");
    Serial.print("Topic: ");
    Serial.println(register_topic_buffer);
  }
  else
  {
    Serial.println("Failed to publish register message.");
  }
  while (!registerCallbackReceived)
  {
    Serial.println("Waiting callback");
    mqtt_client.loop();
    delay(1000);
  }

  return 0;
}

void provisionDevice(char* &iotHubHost, char* &clientId)
{
  initializeProvisioningClient();

  // The SAS token is valid for 1 hour by default in this sample.
  // After one hour the sample must be restarted, or the client won't be able
  // to connect/stay connected to the Azure IoT Hub.
  if (generateSasToken(sas_token, sizeofarray(sas_token)) != 0)
  {
    Serial.println("Failed generating MQTT password");
  }
  else
  {
    connectToAzureProvisioningService();
    mqtt_client.disconnect();
    iotHubHost = _iotHubHost;
    clientId = _clientId; 
  }
}