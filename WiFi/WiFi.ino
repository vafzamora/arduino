#include <WiFi.h>;

const char *ssid = "<your SSID>";
const char *password = "<your password>";
const int LED = 27;

WiFiServer server(80);

void setup()
{
    Serial.begin(115200);
    pinMode(LED, OUTPUT);

    Serial.println("Prepare to connect");
    delay(500);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.println("Connecting to WiFi...");
    }
    Serial.print("Connected to the WiFi network ");
    Serial.println(ssid);
    Serial.print("MAC address: ");
    Serial.println(WiFi.macAddress());
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    server.begin(); 
}

void loop()
{
    WiFiClient client = server.available();
    if (client)
    {
        Serial.println("New client");
        String currentLine = "";
        while (client.connected())
        {
            if (client.available())
            {
                char c = client.read();
                Serial.write(c);
                if (c == '\n')
                {
                    if (currentLine.length() == 0)
                    {
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println();
                        client.print("Click <a href=\"/H\">here</a> to turn the LED on pin 2 on.<br>");
                        client.print("Click <a href=\"/L\">here</a> to turn the LED on pin 2 off.<br>");
                        client.println();
                        break;
                    }
                    else
                    {
                        currentLine = "";
                    }
                }
                else if (c != '\r')
                {
                    currentLine += c;
                }
                if (currentLine.endsWith("GET /H"))
                {
                    digitalWrite(LED, HIGH);
                }
                if (currentLine.endsWith("GET /L"))
                {
                    digitalWrite(LED, LOW);
                }
            }
        }
        client.stop();
        Serial.println("Client Disconnected.");
    }
}