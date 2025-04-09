#ifndef CONFIG_H
#define CONFIG_H

// GPS Neo6M pins
#define GPS_RX_PIN 25 // Connect to TX of GPS module
#define GPS_TX_PIN 26 // Connect to RX of GPS module
#define GPS_BAUD 9600

// SIM800L pins
#define GSM_RX_PIN 32 // Connect to TX of SIM800L module
#define GSM_TX_PIN 33 // Connect to RX of SIM800L module
#define GSM_BAUD 9600

// MQTT Broker settings
#define MQTT_BROKER "u7015b42.ala.asia-southeast1.emqxsl.com"
#define MQTT_PORT 8883
#define MQTT_USERNAME "lokatrack-gps-1"
#define MQTT_PASSWORD "lokatrack"

// WiFi settings
#define WIFI_SSID "QUEENFAM"
#define WIFI_PASSWORD "Q15062009"

#endif