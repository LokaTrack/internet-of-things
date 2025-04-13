/**************************************************************
 * ESP32 GPS Tracker with SIM800L
 *
 * This implementation uses the TinyGSM library to connect
 * to a cellular network using SIM800L module.
 *
 * Configuration is loaded from config.h
 **************************************************************/

// Define SIM800L modem
#define TINY_GSM_MODEM_SIM800
// Increase RX buffer for better stability
#define TINY_GSM_RX_BUFFER 512

#include <Arduino.h>
#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>
#include "config.h"

// Uncomment this if you want to see all AT commands
// #define DUMP_AT_COMMANDS

// Set serial for debug console
#define SerialMon Serial
// Set serial for AT commands
#define SerialAT Serial2

// GPRS credentials
const char apn[] = "internet"; // Your carrier's APN
const char user[] = "wap";     // APN username if needed
const char pass[] = "123wap";  // APN password if needed

// Server details for testing HTTP connection
const char server[] = "vsh.pp.ua";
const char resource[] = "/TinyGSM/logo.txt";

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif

#ifdef MQTT_SSL
TinyGsmClientSecure client(modem);
HttpClient http(client, server, 443);
#else
TinyGsmClient client(modem);
HttpClient http(client, server, 80);
#endif

void setup()
{
  // Set console baud rate
  SerialMon.begin(115200);
  delay(100);

  SerialMon.println("ESP32 GPS Tracker with SIM800L");

  // Set GSM module baud rate and pins
  SerialAT.begin(GSM_BAUD, SERIAL_8N1, GSM_RX_PIN, GSM_TX_PIN);
  delay(1000);

  // Reset the GSM module
  pinMode(GSM_RST_PIN, OUTPUT);
  digitalWrite(GSM_RST_PIN, LOW);
  delay(1000);
  digitalWrite(GSM_RST_PIN, HIGH);
  delay(3000);

  // Initialize modem
  SerialMon.println("Initializing modem...");
  modem.restart();

  String modemInfo = modem.getModemInfo();
  SerialMon.print("Modem Info: ");
  SerialMon.println(modemInfo);

  // Unlock SIM card if needed
  // modem.simUnlock("1234");
}

void loop()
{
  SerialMon.print("Waiting for network...");
  // Print CSQ (Signal Quality Report)
  int csq = modem.getSignalQuality();
  SerialMon.print("Signal quality: ");
  SerialMon.print(csq);
  SerialMon.println(" CSQ");

  if (!modem.waitForNetwork())
  {

    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" success");

  if (modem.isNetworkConnected())
  {
    SerialMon.println("Network connected");
  }

  // GPRS connection parameters
  SerialMon.print("Connecting to ");
  SerialMon.print(apn);
  if (!modem.gprsConnect(apn, user, pass))
  {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" success");

  if (modem.isGprsConnected())
  {
    SerialMon.println("GPRS connected");
  }

  // Test HTTP GET request
  SerialMon.print("Performing HTTP GET request... ");
  int err = http.get(resource);
  if (err != 0)
  {
    SerialMon.println("failed to connect");
    delay(10000);
    return;
  }

  int status = http.responseStatusCode();
  SerialMon.print("Status: ");
  SerialMon.println(status);
  if (!status)
  {
    delay(10000);
    return;
  }

  // Read headers
  SerialMon.println("Headers:");
  while (http.headerAvailable())
  {
    String headerName = http.readHeaderName();
    String headerValue = http.readHeaderValue();
    SerialMon.println(headerName + " : " + headerValue);
  }

  int length = http.contentLength();
  if (length >= 0)
  {
    SerialMon.print("Content length: ");
    SerialMon.println(length);
  }

  if (http.isResponseChunked())
  {
    SerialMon.println("The response is chunked");
  }

  // Read response body
  String body = http.responseBody();
  SerialMon.println("Response body:");
  SerialMon.println(body);
  SerialMon.print("Body length: ");
  SerialMon.println(body.length());

  // Disconnect
  http.stop();
  SerialMon.println("Server disconnected");

  modem.gprsDisconnect();
  SerialMon.println("GPRS disconnected");

  // Wait before next connection attempt
  SerialMon.println("Waiting 30 seconds before next connection...");
  delay(30000);
}
