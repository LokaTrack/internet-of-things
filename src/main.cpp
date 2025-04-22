#include "config.h"

#include <Arduino.h>
#include <TinyGPSPlus.h>
#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// GPS Setup
TinyGPSPlus gps;
HardwareSerial gpsSerial(2); // Use Serial2 as `gpsSerial` for GPS

// GSM Modem Setup
HardwareSerial gsmAtSerial(1); // Use Serial1 as `gsmAtSerial` for AT commands
TinyGsm modem(gsmAtSerial);

// MQTT Client Setup
#ifdef MQTT_SSL
TinyGsmClientSecure gsmClient(modem);
#else
TinyGsmClient gsmClient(modem);
#endif
PubSubClient mqttClient(gsmClient);

uint32_t lastPublishTime = 0;

void publishGpsData();
void connectGprs();

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

  // Initialize GPS module on gpsSerial
  Serial.print("Initializing GPS Serial...");
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  Serial.println("Success!");

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

  Serial.print("Initializing MQTT client...");
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
#ifdef MQTT_SSL
#ifdef MQTT_INSECURE
  // gsmClient.setInsecure(); // Removed: TinyGsmClientSecure doesn't have this method.
  // Serial.println(" (using SSL - Insecure)"); // Comment adjusted
  Serial.println("Sucess! (using SSL - Insecure)");
#else
  // Note: TinyGSM doesn't directly support CA certs like WiFiClientSecure.
  // SSL/TLS is handled by the modem's firmware.
  // Ensure your modem firmware supports the necessary TLS features and ciphers.
  // setCACert is not available for TinyGsmClientSecure.
  Serial.println("Sucess! (using SSL - Secure)");
#endif
#else
  Serial.println("Success! (using non-SSL)");
#endif
}

void loop()
{
  if (!modem.isGprsConnected())
  {
    Serial.println("GPRS disconnected. Reconnecting...");
    connectGprs();
    return;
  }

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

  // Serialize JSON to string
  char buffer[256];
  size_t n = serializeJson(doc, buffer);

  // Publish to MQTT
  Serial.print("Publishing: ");
  Serial.print(buffer);
  if (mqttClient.publish(MQTT_TOPIC, buffer, n))
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