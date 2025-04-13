#include <Arduino.h>
#include <TinyGPSPlus.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "config.h"

// Only include WiFiClientSecure if MQTT_SSL is defined
#ifdef MQTT_SSL
#include <WiFiClientSecure.h>
#endif

TinyGPSPlus gps;
HardwareSerial gpsSerial(2);

// Create either a secure or non-secure WiFi client based on MQTT_SSL
#ifdef MQTT_SSL
WiFiClientSecure wifiClient;
#else
WiFiClient wifiClient;
#endif

PubSubClient mqttClient(wifiClient);

uint32_t lastPublishTime = 0;
char macAddress[18];

void publishGpsData();
void onMqttConnect(bool sessionPresent);
void connectWifi();
void getMacAddress();

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
  getMacAddress();

  Serial.print("Initializing MQTT client...");

  try
  {
    // Configure SSL only if MQTT_SSL is defined
#ifdef MQTT_SSL
#ifdef MQTT_INSECURE
    wifiClient.setInsecure(); // Skip certificate verification
#else
    wifiClient.setCACert(MQTT_CA_CERT); // Use CA certificate for verification
#endif
    Serial.println(" (using SSL)");
#else
    Serial.println(" (not using SSL)");
#endif

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

  delay(1000);
}

void publishGpsData()
{
  Serial.print("Number of satellites: ");
  Serial.println(gps.satellites.value());

  // Create JSON document
  JsonDocument doc;

  // Add device ID
  doc["id"] = macAddress;

  // Add GPS data
  if (gps.location.isValid())
  {
    doc["lat"] = gps.location.lat();
    doc["long"] = gps.location.lng();
  }
  else
  {
    doc["lat"] = 0;
    doc["long"] = 0;
  }

  // Add satellites data
  doc["satellites"] = gps.satellites.value();

  // Serialize JSON to string
  char buffer[256];
  serializeJson(doc, buffer);

  // Publish to MQTT
  mqttClient.publish(MQTT_TOPIC, buffer);
  lastPublishTime = millis();

  // Debug output
  Serial.print("Published: ");
  Serial.print(buffer);
  Serial.print(" - ");
  Serial.print(lastPublishTime);
  Serial.println("ms");
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

void getMacAddress()
{
  uint8_t mac[6];
  WiFi.macAddress(mac);
  snprintf(macAddress, sizeof(macAddress), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.print("MAC Address: ");
  Serial.println(macAddress);
}