#include <HardwareSerial.h>

// Use the default Serial1 port on ESP32
#define SIM800L_RX_PIN 32 // Connect to TX of SIM800L module
#define SIM800L_TX_PIN 33 // Connect to RX of SIM800L module

HardwareSerial SIM800L(1);

void ShowSerialData();
void sendCommand(const char *command);
void sendGetRequest();

// Function to send AT commands to the SIM800L
void sendCommand(const char command)
{
  SIM800L.println(command);
  ShowSerialData();
}

// Function to read and display serial data from the SIM800L
void ShowSerialData()
{
  Serial.println("Show serial data:");
  while (SIM800L.available())
  {
    char c = SIM800L.read();
    Serial.write(c);
  }
  Serial.println("");
  delay(1000);
}

// Function to perform an HTTP GET request using the SIM800L
void sendGetRequest()
{
  sendCommand("AT");
  sendCommand("AT+CIPSHUT");
  sendCommand("AT+SAPBR=0,1");
  sendCommand("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
  sendCommand("AT+SAPBR=3,1,\"APN\",\"internet\""); // Change APN to your network provider's
  sendCommand("AT+SAPBR=1,1");
  sendCommand("AT+HTTPINIT");
  sendCommand("AT+HTTPPARA=\"CID\",1");
  sendCommand("AT+HTTPPARA=\"URL\",\"http://example.com/\""); // Change URL to your desired server
  sendCommand("AT+HTTPACTION=0");
  delay(9000);
  sendCommand("AT+HTTPREAD");
  sendCommand("AT+HTTPTERM");
  sendCommand("AT+CIPSHUT");
  sendCommand("AT+SAPBR=0,1");
}

void setup()
{
  // Start the Serial Monitor on USB
  Serial.begin(9600);
  // Start the Hardware Serial for SIM800L
  SIM800L.begin(9600, SERIAL_8N1, SIM800L_RX_PIN, SIM800L_TX_PIN);

  // Optionally, ensure the SIM800L is properly initialized and ready
  delay(3000);      // Wait for SIM800L to initialize
  sendGetRequest(); // Send the HTTP GET request
}

void loop()
{
  // Keep reading incoming data from SIM800L if present
  if (SIM800L.available())
  {
    Serial.write(SIM800L.read());
  }
  // Add delay to prevent overloading the serial buffer
  delay(100);
}