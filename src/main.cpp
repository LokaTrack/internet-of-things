#include "app_config.h"
#include "modem_config.h"
#include "mqtt_config.h"
#include "pins_config.h"
#include "wifi_config.h"

#include <Arduino.h>
#include <TinyGPSPlus.h>
#ifdef USE_WIFI_CONNECTION
#include <WiFi.h>
#include <WiFiClientSecure.h>
#else
#include <TinyGsmClient.h>
#endif
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ChaCha20.h> // Include the encryption header

// GPS Setup
TinyGPSPlus gps;
HardwareSerial gpsSerial(2); // Use Serial2 as `gpsSerial` for GPS

#ifndef USE_WIFI_CONNECTION
// GSM Modem Setup
HardwareSerial gsmAtSerial(1); // Use Serial1 as `gsmAtSerial` for AT commands
TinyGsm modem(gsmAtSerial);
#endif

// MQTT Client Setup
#ifdef USE_WIFI_CONNECTION
#ifdef MQTT_SSL
WiFiClientSecure wifiClient;
#else
WiFiClient wifiClient;
#endif
PubSubClient mqttClient(wifiClient);
#else
#ifdef MQTT_SSL
TinyGsmClientSecure gsmClient(modem);
#else
TinyGsmClient gsmClient(modem);
#endif
PubSubClient mqttClient(gsmClient);
#endif

uint32_t lastPublishTime = 0;

void publishGpsData();
#ifndef USE_WIFI_CONNECTION
void connectGprs();
#endif
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

  // Initialize the encryption system
  Serial.print("Initializing ChaCha20 encryption...");
  initChaCha();
  Serial.println("Success!");

  // Initialize random seed for secure IV generation
  randomSeed(analogRead(0) + millis());

  // Initialize GPS module on gpsSerial
  Serial.print("Initializing GPS Serial...");
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  Serial.println("Success!");

#ifndef USE_WIFI_CONNECTION
  // Initialize GSM Module on gsmAtSerial
  Serial.print("Initializing GSM Serial...");
  gsmAtSerial.begin(GSM_BAUD, SERIAL_8N1, GSM_RX_PIN, GSM_TX_PIN);
  delay(3000); // Delay for modem stabilization
  Serial.println("Success!");

  Serial.print("Initializing modem...");
  if (!modem.init())
  { // Use init() instead of restart() for initial setup
    Serial.println("Failed!");
    Serial.print("Restarting modem...");
    modem.restart(); // Attempt restart if init fails
    // Consider adding a check here if restart also fails
  }
  Serial.println("Success!");

  connectGprs(); // Connect to GPRS
#else
  connectWifi(); // Connect to WiFi
#endif

  Serial.print("Initializing MQTT client...");
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setBufferSize(1024); // Increase buffer size for large encrypted messages
#ifdef MQTT_SSL
#ifdef USE_WIFI_CONNECTION
#ifdef MQTT_INSECURE
  wifiClient.setInsecure(); // Skip certificate validation
  Serial.println("Success! (using WiFi SSL - Insecure)");
#else
  wifiClient.setCACert(MQTT_CA_CERT); // Set CA certificate for server validation
  Serial.println("Success! (using WiFi SSL - Secure)");
#endif
#else
#ifdef MQTT_INSECURE
  Serial.println("Success! (using GSM SSL - Insecure)");
#else
  Serial.println("Success! (using GSM SSL - Secure)");
#endif
#endif
#else
  Serial.println("Success! (using non-SSL)");
#endif
}

void loop()
{
#ifndef USE_WIFI_CONNECTION
  if (!modem.isGprsConnected())
  {
    Serial.println("GPRS disconnected. Reconnecting...");
    connectGprs();
    return;
  }
#else
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi disconnected. Reconnecting...");
    connectWifi();
    return;
  }
#endif

  if (!mqttClient.connected())
  {
    Serial.print("Connecting to MQTT broker...");
    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD))
    {
      Serial.println("Success!");
    }
    else
    {
      Serial.print("Failed! Error code: ");
      Serial.print(mqttClient.state());
      Serial.println(", Retrying in 5 seconds...");
      delay(5000); // Wait before retrying MQTT connection
    }
    return; // Return to avoid publishing immediately after connection attempt
  }
  mqttClient.loop();

  while (gpsSerial.available() > 0)
  {
    gps.encode(gpsSerial.read());
  }

  if (mqttClient.connected() && (millis() - lastPublishTime > 5000))
  {
    publishGpsData();
  }

  delay(10);
}

#ifndef USE_WIFI_CONNECTION
void connectGprs()
{
  Serial.print("Connecting to GPRS network...");
  if (!modem.waitForNetwork())
  {
    Serial.println("Failed!");
    delay(10000);
    return;
  }
  Serial.println("Success!");

  Serial.print("Connecting to APN: ");
  Serial.print(APN);
  Serial.print("...");
  if (!modem.gprsConnect(APN, APN_USER, APN_PASSWORD))
  {
    Serial.println("Failed!");
    delay(10000);
    return;
  }
  Serial.println("Success!");
}
#endif

#ifdef USE_WIFI_CONNECTION
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
#endif

void publishGpsData()
{
  Serial.print("Number of satellites: ");
  Serial.println(gps.satellites.value());

  // Create JSON document
  JsonDocument doc;

  // Add device ID using MQTT_CLIENT_ID from config.h
  doc["id"] = MQTT_CLIENT_ID;

  // Add GPS data
  if (gps.location.isValid())
  {
    doc["lat"] = gps.location.lat();
    doc["long"] = gps.location.lng();
  }
  else
  {
    doc["lat"] = nullptr;
    doc["long"] = nullptr;
  }

  // Add satellites data
  doc["satellites"] = gps.satellites.value();

  // Add HDOP (Horizontal Dilution of Precision) data
  if (gps.hdop.isValid())
  {
    doc["hdop"] = gps.hdop.hdop();
  }
  else
  {
    doc["hdop"] = nullptr;
  }

  // Add altitude data
  if (gps.altitude.isValid())
  {
    doc["alt"] = gps.altitude.meters();
  }
  else
  {
    doc["alt"] = nullptr;
  }

  // Add speed data
  if (gps.speed.isValid())
  {
    doc["speed"] = gps.speed.kmph();
  }
  else
  {
    doc["speed"] = nullptr;
  }

#ifdef USE_DUMMY_GPS_DATA
  doc["dummy"] = true;
#else
  doc["dummy"] = false;
#endif

  // Get plain JSON for debug
  String plainJson;
  serializeJson(doc, plainJson);
  Serial.print("Plain JSON: ");
  Serial.println(plainJson);

  // Encrypt the JSON document - this will automatically include IV and counter in the output
  String encryptedData = encryptJson(doc);

  // Publish encrypted data to MQTT
  Serial.print("Publishing encrypted data (length: ");
  Serial.print(encryptedData.length());
  Serial.print(" bytes)");

  if (mqttClient.publish(MQTT_TOPIC, encryptedData.c_str()))
  {
    Serial.print(" - ");
    Serial.print(millis());
    Serial.print(" ms...");
    lastPublishTime = millis();
    Serial.println("Success!");
  }
  else
  {
    Serial.print(" - ");
    Serial.print(millis());
    Serial.print(" ms...");
    Serial.println("Failed!");
  }
}
