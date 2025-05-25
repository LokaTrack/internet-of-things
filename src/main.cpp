#include "pins_config.h"
#include <TinyGPS++.h>
#include <HardwareSerial.h>

// GPS objects
TinyGPSPlus gps;
HardwareSerial gpsSerial(1); // Use UART1 for GPS

void setup()
{
  // Initialize Serial Monitor
  Serial.begin(9600);
  Serial.println("LokaTrack-IoT GPS Neo M8N Initialization");

  // Initialize GPS Serial communication
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

  Serial.println("GPS Neo M8N connected on pins:");
  Serial.println("GPS RX Pin: " + String(GPS_RX_PIN));
  Serial.println("GPS TX Pin: " + String(GPS_TX_PIN));
  Serial.println("GPS Baud Rate: " + String(GPS_BAUD));
  Serial.println("Waiting for GPS signal...");
}

void loop()
{
  // Read GPS data
  while (gpsSerial.available() > 0)
  {
    char c = gpsSerial.read();

    if (gps.encode(c))
    {
      // Check if we have a valid location
      if (gps.location.isValid())
      {
        Serial.println("=== GPS Data ===");
        Serial.print("Latitude: ");
        Serial.println(gps.location.lat(), 6);
        Serial.print("Longitude: ");
        Serial.println(gps.location.lng(), 6);

        if (gps.altitude.isValid())
        {
          Serial.print("Altitude: ");
          Serial.print(gps.altitude.meters());
          Serial.println(" meters");
        }

        if (gps.speed.isValid())
        {
          Serial.print("Speed: ");
          Serial.print(gps.speed.kmph());
          Serial.println(" km/h");
        }

        if (gps.satellites.isValid())
        {
          Serial.print("Satellites: ");
          Serial.println(gps.satellites.value());
        }

        if (gps.hdop.isValid())
        {
          Serial.print("HDOP: ");
          Serial.println(gps.hdop.hdop());
        }

        if (gps.date.isValid() && gps.time.isValid())
        {
          Serial.print("Date/Time: ");
          Serial.printf("%02d/%02d/%04d %02d:%02d:%02d\n",
                        gps.date.day(), gps.date.month(), gps.date.year(),
                        gps.time.hour(), gps.time.minute(), gps.time.second());
        }

        Serial.println("================");
        delay(2000); // Print GPS data every 2 seconds when available
      }
    }
  }

  // Check if GPS is connected but no data
  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println("No GPS data received: check wiring or GPS antenna");
    delay(5000);
  }
}
