#if !defined(MODEM_CONFIG_H)
#define MODEM_CONFIG_H)

// GSM modem settings
const char APN[] = "internet";        // Your carrier's APN
const char APN_USER[] = "wap";        // APN username if needed
const char APN_PASSWORD[] = "123wap"; // APN password if needed

// Type of GSM module
#define TINY_GSM_MODEM_SIM800
// #define TINY_GSM_MODEM_SIM808
// #define TINY_GSM_MODEM_SIM868
// #define TINY_GSM_MODEM_SIM7000SSL
// #define TINY_GSM_MODEM_SIM7080
// #define TINY_GSM_MODEM_UBLOX
// #define TINY_GSM_MODEM_SARAR4
// #define TINY_GSM_MODEM_ESP8266
// #define TINY_GSM_MODEM_ESP32
// #define TINY_GSM_MODEM_XBEE
// #define TINY_GSM_MODEM_SEQUANS_MONARCH

#endif // MODEM_CONFIG_H)
