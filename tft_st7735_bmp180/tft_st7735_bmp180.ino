#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <WiFi.h>

#include "iot_config.h"

//ESP32
#define TFT_CS 5
#define TFT_RST 4
#define TFT_RS 2
#define TFT_CLK 18
#define TFT_SDA 23

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_RS, TFT_RST);
const int BARS = 8;

Adafruit_BMP085 bmp;

uint16_t colors[BARS] = {
    ST7735_BLACK,
    ST7735_BLUE,
    ST7735_RED,
    ST7735_GREEN,
    ST7735_CYAN,
    ST7735_MAGENTA,
    ST7735_YELLOW,
    ST7735_WHITE
};

struct bmpData{
  float temperature;
  float pressure;
};

void drawBARS()
{
  int16_t barWidth = tft.width() / BARS;
  int16_t barHeight = tft.height();

  uint16_t x0 = 0;
  uint16_t y0 = 0;

  for (int i = 0; i < BARS; i++)
  {
    tft.fillRect(x0, y0, barWidth, barHeight, colors[i]);
    x0 += barWidth;
  }
}

bmpData readBmpSensor()
{
  bmpData data;
  data.temperature = bmp.readTemperature();
  data.pressure = bmp.readPressure();

  Serial.print("Temperature = ");
  Serial.print(data.temperature);
  Serial.println(" *C");

  Serial.print("Pressure = ");
  Serial.print(data.pressure);
  Serial.println(" Pa");
  return data;
}

void setupWifi()
{
  WiFi.begin(IOT_CONFIG_WIFI_SSID, IOT_CONFIG_WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.print("Connected to the WiFi network ");
  Serial.println(IOT_CONFIG_WIFI_SSID);
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void clearScreen(){
  tft.fillScreen(ST7735_BLACK);
  tft.drawRect(0, 0, tft.width(), 22, ST7735_CYAN);
}

void displayWiFiInfo(){
  tft.setCursor(2,24); 
  tft.print(IOT_CONFIG_WIFI_SSID);
  tft.setCursor(2,34);
  tft.print(WiFi.localIP());
  tft.setCursor(2,44);
  tft.print(WiFi.macAddress());
}

void setup()
{
  Serial.begin(9600);

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(2); //180°
  drawBARS();
  setupWifi();

  if (!bmp.begin())
  {
    Serial.println("Could not find a valid sensor!");
  }
  clearScreen(); 
  displayWiFiInfo();
}

void loop()
{
  bmpData data = readBmpSensor();

  tft.fillRect(1, 1, tft.width() - 2, 20, ST7735_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(ST7735_YELLOW);

  tft.setCursor(2, 2);
  tft.print("Temp: ");
  tft.print(data.temperature, 2);
  tft.print(" C");

  tft.setCursor(2, 12);
  tft.print("Press: ");
  tft.print(data.pressure, 2);
  tft.print(" Pa");

  delay(2000);
}
