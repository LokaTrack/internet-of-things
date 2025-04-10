#include <Arduino.h>
#include <TinyGPSPlus.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "config.h"

TinyGPSPlus gps;
HardwareSerial gpsSerial(2);
WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);

uint32_t lastPublishTime = 0;

void publishGpsData();
void onMqttConnect(bool sessionPresent);
void connectWifi();

void setup()
{
  // Initialize Serial Monitor
  Serial.begin(9600);
  Serial.println();
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
    wifiClient.setInsecure();
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  }
  catch (const std::exception &e)
  {
    Serial.println("Failed!");
    Serial.println(e.what());
  }
  Serial.println("Success!");

  Serial.print("Connecting to MQTT broker...");
  mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD);
  while (!mqttClient.connected())
  {
    mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD);
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Success!");
}

void loop()
{
  if (!mqttClient.connected())
  {
    Serial.println("Reconnecting to MQTT broker...");
    mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD);
    return;
  }

  if (millis() - lastPublishTime < 5000)
  {
    return;
  }

  while (gpsSerial.available() > 0)
  {
    gps.encode(gpsSerial.read());
  }

  publishGpsData();

  // Serial.print("Satellites: ");
  // Serial.println(gps.satellites.value());
  // lastPublishTime = millis();

  delay(1000);
}

void publishGpsData()
{
  if (gps.location.isValid())
  {
    String payload = "{\"lat\": " + String(gps.location.lat(), 6) + ", \"long\": " + String(gps.location.lng(), 6) + "}";
    mqttClient.publish(MQTT_TOPIC, payload.c_str());
    lastPublishTime = millis();
    Serial.print("Published: ");
    Serial.print(payload);
    Serial.print(" - ");
    Serial.print(lastPublishTime);
    Serial.println("ms");
  }
  else
  {
    // Dummy payload to indicate no GPS fix
    String payload = "{\"lat\": 0, \"long\": 0}";
    mqttClient.publish(MQTT_TOPIC, payload.c_str());
    lastPublishTime = millis();
    Serial.print("Published: ");
    Serial.print(payload);
    Serial.print(" - ");
    Serial.print(lastPublishTime);
    Serial.println("ms");
  }
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