#include <Arduino.h>
#include <TinyGPSPlus.h>
#include <AsyncMqttClient.h>
#include <WiFi.h>
#include "config.h"

TinyGPSPlus gps;
HardwareSerial gpsSerial(2);
AsyncMqttClient mqttClient;

void onMqttConnect(bool sessionPresent);
void connectWifi();

void setup()
{
  // Initialize Serial Monitor
  Serial.begin(9600);
  Serial.print("Initializing Serial Monitor...");
  while (!Serial)
  {
    ; // Wait for serial port to connect
  }
  Serial.println("Success!");

  // Initialize GPS module on Serial2
  Serial.print("Initializing GPS Serial...");
  try
  {
    gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  }
  catch (const std::exception &e)
  {
    Serial.println("Failed!");
    Serial.println(e.what());
  }
  Serial.println("Success!");

  connectWifi();

  Serial.print("Initializing MQTT client...");
  try
  {
    mqttClient.onConnect(onMqttConnect);
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  }
  catch (const std::exception &e)
  {
    Serial.println("Failed!");
    Serial.println(e.what());
  }
  Serial.println("Success!");

  Serial.print("Connecting to MQTT broker...");
  mqttClient.connect();
}

void loop()
{
  // Read data from GPS module and feed it to TinyGPS++ library
  while (gpsSerial.available() > 0)
  {
    gps.encode(gpsSerial.read());
  }

  // Display GPS status and data every second
  Serial.println("-------------------------------");

  if (gps.location.isValid())
  {
    Serial.print("Location: ");
    Serial.print(gps.location.lat(), 6);
    Serial.print(", ");
    Serial.println(gps.location.lng(), 6);
  }
  else
  {
    Serial.println("Waiting for GPS fix...");
  }

  Serial.print("Satellites: ");
  Serial.println(gps.satellites.value());

  delay(1000);
}

void onMqttConnect(bool sessionPresent)
{
  Serial.println("Success!");
}

void connectWifi()
{
  Serial.print("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Success!");
}