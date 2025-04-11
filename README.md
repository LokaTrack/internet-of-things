# LokaTrack Internet of Things

This repository hosts the source code for the LokaTrack IoT project. This project is designed to track the location of a driver using a GPS module and send the data to a server using MQTT over WiFi.

## Table of Contents

- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Software Requirements](#software-requirements)
- [Installation](#installation)
- [Configuration](#configuration)
- [Usage](#usage)
- [Contributing](#contributing)
- [License](#license)

## Features

- Real-time GPS tracking with NEO-6M module
- WiFi connectivity for data transmission
- Secure or non-secure MQTT communication based on configuration
- Flexible configuration options for different deployment scenarios
- Automatic reconnection to WiFi and MQTT broker
- Unique device identification using MAC address

## Hardware Requirements

- ESP32 development board
- NEO-6M GPS module
- Micro USB cable for programming and power
- Jumper wires
- Breadboard (for prototyping) or PCB (for final product)
- GPS antenna

## Software Requirements

- [PlatformIO IDE](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide) with [pioarduino extension](https://marketplace.visualstudio.com/items?itemName=pioarduino.pioarduino-ide) or Arduino IDE
- [Espressif ESP32 Dev Module](https://docs.platformio.org/en/latest/boards/espressif32/esp32dev.html?utm_source=platformio&utm_medium=piohome) board support package
- Libraries:
  - [TinyGPS++](https://github.com/mikalhart/TinyGPSPlus?utm_source=platformio&utm_medium=piohome) (for GPS data parsing)
  - [PubSubClient](https://github.com/knolleary/pubsubclient?utm_source=platformio&utm_medium=piohome) (for MQTT communication)

## Installation

Clone this repository to your local machine:

```bash
git clone https://github.com/LokaTrack/internet-of-things.git
```

Then, open Visual Studio Code and open the cloned repository. Install PlatformIO IDE and pioarduino extension into Visual Studio Code. After that, restart Visual Studio Code and wait for PlatformIO to install the necessary dependencies.

## Configuration

All configuration can be found in the `include/config.h` file. The project supports flexible configuration options:

### GPS Configuration

- GPS_RX_PIN: 25 (Connect to TX of GPS module)
- GPS_TX_PIN: 26 (Connect to RX of GPS module)
- GPS_BAUD: 9600

### MQTT Configuration

- MQTT_BROKER: "u7015b42.ala.asia-southeast1.emqxsl.com"
- MQTT_TOPIC: "lokatrack/gps"
- MQTT_PORT: 8883 (Secure connection)
- MQTT_CLIENT_ID: "lokatrack-gps-1"
- MQTT_USERNAME: "lokatrack-gps-1"
- MQTT_PASSWORD: "lokatrack"

### Security Options

The code supports three security modes controlled by defining or commenting out these options in `config.h`:

1. **Secure mode with certificate verification:**

   ```cpp
   #define MQTT_SSL
   // #define MQTT_INSECURE (commented out)
   ```

2. **Secure mode without certificate verification:**

   ```cpp
   #define MQTT_SSL
   #define MQTT_INSECURE
   ```

3. **Non-secure mode:**
   ```cpp
   // #define MQTT_SSL (commented out)
   // #define MQTT_INSECURE (commented out)
   ```

Your current configuration uses secure mode without certificate verification (option 2).

### WiFi Settings

- WIFI_SSID: "QUEENFAM"
- WIFI_PASSWORD: "Q15062009"

### Additional Configuration

There are also pins defined for SIM800L GSM module, but they are not currently used in the code:

- GSM_RX_PIN: 32
- GSM_TX_PIN: 33
- GSM_BAUD: 9600

## Usage

After uploading the code to your ESP32, the device will:

1. Connect to the configured WiFi network
2. Get the device's MAC address for unique identification
3. Connect to the MQTT broker using the configured security settings
4. Begin reading GPS data and publishing it to the MQTT topic every 5 seconds

The device will automatically reconnect to WiFi and the MQTT broker if the connection is lost.

## License

[Add your license information here]
