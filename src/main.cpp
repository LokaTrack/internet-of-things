#include <Arduino.h>
#include <TinyGPSPlus.h>
#include "config.h"

// Create a TinyGPS++ object
TinyGPSPlus gps;

// Using Hardware Serial 2 for GPS communication
HardwareSerial gpsSerial(2); // UART 2

void setup()
{
  // Initialize Serial Monitor
  Serial.begin(9600);
  while (!Serial)
  {
    ; // Wait for serial port to connect
  }

  // Initialize GPS module on Serial2
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RX_PIN, TX_PIN);

  Serial.println("ESP32 GPS Neo6M Initialization");
  Serial.println("GPS Serial started at 9600 baud rate");
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
