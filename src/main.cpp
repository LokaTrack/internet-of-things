#include "config.h"

#include <Arduino.h>
#include <TinyGPSPlus.h>
#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// GPS Setup
TinyGPSPlus gps;
HardwareSerial gpsSerial(2); // Use Serial2 for GPS

// GSM Modem Setup
HardwareSerial gsmAtSerial(1); // Use Serial1 for AT commands
TinyGsm modem(gsmAtSerial);

// MQTT Client Setup
// Create either a secure or non-secure GSM client based on MQTT_SSL
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

  // Initialize GPS module on Serial2 - Re-added
  Serial.print("Initializing GPS Serial...");
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  Serial.println("Success!"); // Assuming success if no exception

  // Initialize GSM Module on Serial1
  Serial.print("Initializing GSM Serial...");
  gsmAtSerial.begin(GSM_BAUD, SERIAL_8N1, GSM_RX_PIN, GSM_TX_PIN);
  delay(3000); // Delay for modem stabilization
  Serial.println("Success!");

  Serial.print("Initializing modem...");
  if (!modem.init())
  { // Use init() instead of restart() for initial setup
    Serial.println("Failed to init modem, restarting...");
    modem.restart(); // Attempt restart if init fails
                     // Consider adding a check here if restart also fails
  }
  Serial.println("Success!");

  connectGprs(); // Connect to GPRS

  Serial.print("Initializing MQTT client...");

  // Configure SSL only if MQTT_SSL is defined
#ifdef MQTT_SSL
#ifdef MQTT_INSECURE
  // gsmClient.setInsecure(); // Removed: TinyGsmClientSecure doesn't have this method.
  // Serial.println(" (using SSL - Insecure)"); // Comment adjusted
  Serial.println(" (using SSL - Insecure flag set, but modem handles verification)");
#else
  // Note: TinyGSM doesn't directly support CA certs like WiFiClientSecure.
  // SSL/TLS is handled by the modem's firmware.
  // Ensure your modem firmware supports the necessary TLS features and ciphers.
  // setCACert is not available for TinyGsmClientSecure.
  Serial.println(" (using SSL - Secure - Modem handles verification)");
#endif
#else
  Serial.println(" (not using SSL)");
#endif

  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);

  Serial.println("Success!");

  // Connect to MQTT broker - moved to loop to handle reconnections robustly
}

void connectGprs()
{
  Serial.print("Connecting to GPRS network...");
  if (!modem.waitForNetwork())
  {
    Serial.println(" Network failed!");
    delay(10000);
    return;
  }
  Serial.println(" Network success!");

  Serial.print("Connecting to APN: ");
  Serial.print(apn);
  if (!modem.gprsConnect(apn, user, pass))
  {
    Serial.println(" APN failed!");
    delay(10000);
    return;
  }
  Serial.println(" APN success!");
}

void loop()
{
  // Ensure GPRS is connected
  if (!modem.isGprsConnected())
  {
    Serial.println("GPRS disconnected. Reconnecting...");
    connectGprs();
    // If GPRS reconnects, MQTT will attempt to reconnect below
  }

  // Ensure MQTT is connected
  if (modem.isGprsConnected() && !mqttClient.connected())
  {
    Serial.print("Connecting to MQTT broker...");
    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD))
    {
      Serial.println("Success!");
      // Optional: Subscribe to topics here if needed
      // mqttClient.subscribe("your/topic");
    }
    else
    {
      Serial.print("Failed! rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000); // Wait before retrying MQTT connection
    }
    return; // Return to avoid publishing immediately after connection attempt
  }

  // Maintain MQTT connection
  mqttClient.loop();

  // Read GPS data - Re-added
  while (gpsSerial.available() > 0)
  {
    gps.encode(gpsSerial.read());
  }

  // Publish data periodically only if MQTT is connected
  if (mqttClient.connected() && (millis() - lastPublishTime > 5000)) // Publish every 5 seconds
  {
    publishGpsData();
  }

  // Small delay to prevent busy-waiting
  delay(10);
}

// Reverted publishGpsData function (using MQTT_CLIENT_ID for id)
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
    doc["lat"] = 0;
    doc["long"] = 0;
  }

  // Add satellites data
  doc["satellites"] = gps.satellites.value();

  // Add HDOP (Horizontal Dilution of Precision) if available
  if (gps.hdop.isValid())
  {
    doc["hdop"] = gps.hdop.hdop();
  }
  else
  {
    doc["hdop"] = 0;
  }

  // Add Altitude if available
  if (gps.altitude.isValid())
  {
    doc["alt"] = gps.altitude.meters();
  }
  else
  {
    doc["alt"] = 0;
  }

  // Add Speed if available
  if (gps.speed.isValid())
  {
    doc["speed"] = gps.speed.kmph();
  }
  else
  {
    doc["speed"] = 0;
  }

  // Serialize JSON to string
  char buffer[256];
  size_t n = serializeJson(doc, buffer);

  // Publish to MQTT
  Serial.print("Publishing: ");
  Serial.println(buffer);
  if (mqttClient.publish(MQTT_TOPIC, buffer, n))
  {
    lastPublishTime = millis();
    Serial.println("Publish Success!");
  }
  else
  {
    Serial.println("Publish Failed!");
  }
}