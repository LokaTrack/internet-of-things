#ifndef CONFIG_H
#define CONFIG_H

// GPS Neo6M pins
#define GPS_RX_PIN 25 // Connect to TX of GPS module
#define GPS_TX_PIN 26 // Connect to RX of GPS module
#define GPS_BAUD 9600

// SIM800L pins
#define GSM_RX_PIN 32  // Connect to TX of SIM800L module
#define GSM_TX_PIN 33  // Connect to RX of SIM800L module
#define GSM_RST_PIN 12 // Connect to RST of SIM800L module
#define GSM_BAUD 9600
const char apn[] = "internet"; // Your carrier's APN
const char user[] = "wap";     // APN username if needed
const char pass[] = "123wap";  // APN password if needed

// MQTT Broker settings
#define MQTT_BROKER "broker.emqx.io"
#define MQTT_TOPIC "lokatrack/gps"
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "lokatrack-gps-1"
#define MQTT_USERNAME "lokatrack-gps-1"
#define MQTT_PASSWORD "lokatrack"
// #define MQTT_SSL // Disabled SSL
// #define MQTT_INSECURE // Disabled as SSL is off

// const char MQTT_CA_CERT[] PROGMEM = R"EOF(
// -----BEGIN CERTIFICATE-----
// MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh
// MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
// d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD
// QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT
// MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
// b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG
// 9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB
// CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97
// nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt
// 43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P
// T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4
// gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO
// BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR
// TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw
// DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr
// hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg
// 06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF
// PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls
// YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk
// CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=
// -----END CERTIFICATE-----
// )EOF";

// WiFi settings
#define WIFI_SSID "QUEENFAM"
#define WIFI_PASSWORD "Q15062009"

#endif