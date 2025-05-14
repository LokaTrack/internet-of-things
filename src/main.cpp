#include "app_config.h"
#include "modem_config.h"
#include "mqtt_config.h"
#include "pins_config.h"
#include "wifi_config.h"
#include "ntp_config.h"

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
#include <ChaCha20.h>  // Include the encryption header
#include <ESP32Time.h> // Include the RTC library

// GPS Setup
TinyGPSPlus gps;
HardwareSerial gpsSerial(2); // Use Serial2 as `gpsSerial` for GPS

// RTC Setup
ESP32Time rtc(GMT_OFFSET);

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
void syncNtpTime();
#endif
void connectWifi();

// NTP Time sync function for SIM800L
#ifndef USE_WIFI_CONNECTION
void syncNtpTime()
{
  Serial.print("Synchronizing time with NTP server: ");
  Serial.print(NTP_SERVER);
  Serial.print("...");

  // First make sure the GPRS is connected
  if (!modem.isGprsConnected())
  {
    Serial.println("GPRS not connected. Cannot sync time.");
    return;
  }

  // SIM800L specific AT commands for NTP sync
  gsmAtSerial.println("AT+CNTPCID=1"); // Set PDP context for NTP
  delay(1000);

  // Clear any pending responses
  while (gsmAtSerial.available())
  {
    gsmAtSerial.read();
  }

  // Set NTP server address and time zone
  String ntpCommand = "AT+CNTP=\"" + String(NTP_SERVER) + "\"," + String(GMT_OFFSET / 3600);
  gsmAtSerial.println(ntpCommand);
  delay(1000);

  // Read response
  String response = "";
  unsigned long timeout = millis() + 5000; // 5 second timeout
  while (millis() < timeout)
  {
    if (gsmAtSerial.available())
    {
      char c = gsmAtSerial.read();
      response += c;
      if (response.indexOf("OK") != -1)
      {
        break;
      }
    }
  }

  if (response.indexOf("OK") == -1)
  {
    Serial.println("Failed to set NTP server!");
    return;
  }

  // Request time synchronization
  gsmAtSerial.println("AT+CNTP");
  delay(1000);

  // Read response for +CNTP: 1 which indicates success
  response = "";
  timeout = millis() + 10000; // 10 second timeout
  while (millis() < timeout)
  {
    if (gsmAtSerial.available())
    {
      char c = gsmAtSerial.read();
      response += c;
      if (response.indexOf("+CNTP: 1") != -1)
      {
        break;
      }
    }
  }

  if (response.indexOf("+CNTP: 1") == -1)
  {
    Serial.println("Failed to sync time!");
    return;
  }

  // Get the network time
  gsmAtSerial.println("AT+CCLK?");
  delay(1000);

  // Read response with format +CCLK: "YY/MM/DD,HH:MM:SS±ZZ"
  response = "";
  timeout = millis() + 5000;
  while (millis() < timeout)
  {
    if (gsmAtSerial.available())
    {
      char c = gsmAtSerial.read();
      response += c;
      if (response.indexOf("+CCLK:") != -1 && response.indexOf("OK") != -1)
      {
        break;
      }
    }
  }

  // Parse the time from response
  int cclkIndex = response.indexOf("+CCLK: \"");
  if (cclkIndex != -1)
  {
    // Extract the time string
    String timeStr = response.substring(cclkIndex + 8, response.indexOf("\"", cclkIndex + 8));

    // Parse the time components (YY/MM/DD,HH:MM:SS±ZZ)
    int year = 2000 + timeStr.substring(0, 2).toInt(); // Convert YY to YYYY
    int month = timeStr.substring(3, 5).toInt();
    int day = timeStr.substring(6, 8).toInt();
    int hour = timeStr.substring(9, 11).toInt();
    int minute = timeStr.substring(12, 14).toInt();
    int second = timeStr.substring(15, 17).toInt();

    // Set the ESP32 RTC
    rtc.setTime(second, minute, hour, day, month, year);

    Serial.println("Success!");
    Serial.print("Current time: ");
    Serial.print(rtc.getTime("%Y-%m-%d %H:%M:%S"));
    Serial.println(" UTC");
  }
  else
  {
    Serial.println("Failed to get time!");
  }
}
#endif

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

  // Synchronize time with NTP server
  Serial.println("Synchronizing time with NTP server...");
  syncNtpTime();
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

  if (mqttClient.connected() && (millis() - lastPublishTime > PUBLISH_INTERVAL))
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

  // Add timestamp with ISO 8601 format including milliseconds
  // Format: YYYY-MM-DDThh:mm:ss.sssZ for UTC
  String isoTimestamp = rtc.getTime("%Y-%m-%dT%H:%M:%S");
  // Add milliseconds (approximate)
  unsigned long msInThisSecond = millis() % 1000;
  char msStr[5];
  sprintf(msStr, ".%03lu", msInThisSecond);
  isoTimestamp += msStr;
  isoTimestamp += "Z"; // 'Z' indicates UTC timezone

  doc["timestamp"] = isoTimestamp;

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
