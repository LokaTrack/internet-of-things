/**************************************************************
 * ESP32 AT Command Interface for SIM800L
 *
 * This implementation allows sending custom AT commands to the
 * SIM800L module and view responses directly via Serial Monitor.
 *
 * Configuration is loaded from config.h
 **************************************************************/

#include <Arduino.h>
#include "config.h"

// Set serial for debug console and user input
#define SerialMon Serial
// Set serial for AT commands (SIM800L)
#define SerialAT Serial2

String inputString = "";     // String to hold incoming data
bool stringComplete = false; // Flag for completed command

void setup()
{
  // Initialize serial ports
  SerialMon.begin(115200);
  SerialAT.begin(GSM_BAUD, SERIAL_8N1, GSM_RX_PIN, GSM_TX_PIN);
  // Reset the GSM module
  pinMode(GSM_RST_PIN, OUTPUT);
  digitalWrite(GSM_RST_PIN, LOW);
  delay(1000);
  digitalWrite(GSM_RST_PIN, HIGH);
  delay(3000);

  SerialMon.println("\n--- ESP32 AT Command Interface for SIM800L ---");
  SerialMon.println("Type AT commands and press enter to send to the modem");
  SerialMon.println("Examples:");
  SerialMon.println("  AT - Basic attention command");
  SerialMon.println("  AT+CSQ - Check signal quality");
  SerialMon.println("  AT+COPS? - Check network operator");
  SerialMon.println("  AT+CREG? - Check network registration");
  SerialMon.println("  AT+CGATT? - Check GPRS attachment");
  SerialMon.println("----------------------------------------");

  // Reserve 200 bytes for the inputString
  inputString.reserve(200);
}

void loop()
{
  // If there's data from the modem, forward it to the Serial Monitor
  if (SerialAT.available())
  {
    SerialMon.write(SerialAT.read());
  }

  // If a command has been completed (Enter pressed)
  if (stringComplete)
  {
    // Send the command to the modem
    SerialMon.print("\r\n> Sending: ");
    SerialMon.println(inputString);
    SerialAT.println(inputString);

    // Clear the string for the next command
    inputString = "";
    stringComplete = false;
  }
}

// SerialEvent occurs whenever new data comes in the hardware serial RX
void serialEvent()
{
  while (SerialMon.available())
  {
    // Get the new byte
    char inChar = (char)SerialMon.read();

    // If the incoming character is a newline, set a flag so the main loop can
    // process the command
    if (inChar == '\n' || inChar == '\r')
    {
      stringComplete = true;
    }
    else
    {
      // Add the character to the inputString
      inputString += inChar;
    }
  }
}
