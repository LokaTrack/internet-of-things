# LokaTrack Internet of Things

This repository hosts the source code for the LokaTrack IoT project. This project is designed to track the location of a driver using a GPS module and send the data to a server using encrypted MQTT over WiFi or GSM.

## Table of Contents

- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Software Requirements](#software-requirements)
- [Installation](#installation)
- [Configuration](#configuration)
- [Usage](#usage)
- [Security](#security)
- [Contributing](#contributing)
- [License](#license)

## Features

- Real-time GPS tracking with NEO-6M module
- WiFi or GSM connectivity for data transmission
- ChaCha20 encryption for securing GPS data
- Secure or non-secure MQTT communication based on configuration
- Flexible configuration options for different deployment scenarios
- Automatic reconnection to WiFi/GSM and MQTT broker
- Unique device identification using MAC address
- Configurable MQTT buffer size for handling larger encrypted messages

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
  - [ArduinoJson](https://github.com/bblanchon/ArduinoJson?utm_source=platformio&utm_medium=piohome) (for JSON data formatting)
  - [TinyGSM](https://github.com/vshymanskyy/TinyGSM) (for GSM communication)
  - [Crypto](https://github.com/rweather/Crypto) (for ChaCha20 encryption)

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

The device will automatically reconnect to WiFi and the MQTT broker if the connection is lost. The GPS data will be published as a JSON string with the following format:

```json
{
  "id":"MQTT_CLIENT_ID",
  "lat": LATITUDE | null,
  "long": LONGITUDE | null,
  "satellites": SATELLITES | null,
  "hdop": HORIZONTAL DILUTION | null,
  "alt": ALTITUDE | null,
  "speed": SPEED | null,
  "dummy": "USE_DUMMY_GPS_DATA"
}
```

## Security

### ChaCha20 Encryption

The LokaTrack IoT device uses ChaCha20 encryption to secure the GPS data before transmission. ChaCha20 is a high-speed cipher that provides strong security and is well-suited for embedded devices.

#### How the Encryption Works

1. The device generates a random 8-byte IV (Initialization Vector) for each message
2. The device uses a fixed 8-byte counter that increments for each block
3. The message format is: `[IV(8 bytes)][Counter(8 bytes)][Encrypted Data(variable)]`
4. All data is encrypted using a pre-shared 32-byte key defined in the code

#### Decrypting Messages

To decrypt messages on your server, you'll need a compatible ChaCha20 implementation. Here's a Python example using the custom implementation that matches our embedded device's encryption:

```python
import json
import binascii
import logging
from Crypto.Util.strxor import strxor
import struct

logger = logging.getLogger(__name__)

DEFAULT_KEY = bytes([
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
    0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0,
    0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8,
])

def decrypt_message(encrypted_hex_message, key=DEFAULT_KEY):
    def quarter_round(state, a, b, c, d):
        """ChaCha20 quarter round function"""
        state[a] = (state[a] + state[b]) & 0xFFFFFFFF
        state[d] ^= state[a]
        state[d] = ((state[d] << 16) | (state[d] >> 16)) & 0xFFFFFFFF

        state[c] = (state[c] + state[d]) & 0xFFFFFFFF
        state[b] ^= state[c]
        state[b] = ((state[b] << 12) | (state[b] >> 20)) & 0xFFFFFFFF

        state[a] = (state[a] + state[b]) & 0xFFFFFFFF
        state[d] ^= state[a]
        state[d] = ((state[d] << 8) | (state[d] >> 24)) & 0xFFFFFFFF

        state[c] = (state[c] + state[d]) & 0xFFFFFFFF
        state[b] ^= state[c]
        state[b] = ((state[b] << 7) | (state[b] >> 25)) & 0xFFFFFFFF

        return state

    def chacha20_block(key, counter_value, nonce):
        """Generate a ChaCha20 block"""
        # Constants for ChaCha20
        CONSTANTS = [0x61707865, 0x3320646E, 0x79622D32, 0x6B206574]

        # Create initial state
        state = CONSTANTS[:]

        # Add key words (8 for 256-bit key)
        for i in range(8):
            state.append(struct.unpack("<I", key[i * 4 : i * 4 + 4])[0])

        # Add counter and nonce
        state.append(counter_value & 0xFFFFFFFF)
        state.append((counter_value >> 32) & 0xFFFFFFFF)
        state.append(struct.unpack("<I", nonce[:4])[0])
        state.append(struct.unpack("<I", nonce[4:8])[0])

        # Copy initial state
        working_state = state[:]

        # ChaCha20 rounds (20 rounds = 10 iterations of double round)
        for _ in range(10):
            # Column round
            working_state = quarter_round(working_state, 0, 4, 8, 12)
            working_state = quarter_round(working_state, 1, 5, 9, 13)
            working_state = quarter_round(working_state, 2, 6, 10, 14)
            working_state = quarter_round(working_state, 3, 7, 11, 15)

            # Diagonal round
            working_state = quarter_round(working_state, 0, 5, 10, 15)
            working_state = quarter_round(working_state, 1, 6, 11, 12)
            working_state = quarter_round(working_state, 2, 7, 8, 13)
            working_state = quarter_round(working_state, 3, 4, 9, 14)

        # Add working state to initial state
        for i in range(16):
            state[i] = (state[i] + working_state[i]) & 0xFFFFFFFF

        # Convert state to bytes
        result = bytearray(64)
        for i in range(16):
            struct.pack_into("<I", result, i * 4, state[i])

        return bytes(result)

    try:
        encrypted_message = binascii.unhexlify(encrypted_hex_message)

        iv = encrypted_message[:8]
        counter_bytes = encrypted_message[8:16]
        ciphertext = encrypted_message[16:]

        # Get the starting counter value as a 64-bit integer
        counter_value = int.from_bytes(counter_bytes, byteorder='little')

        # Create an empty buffer for the keystream
        keystream = bytearray()

        # Generate keystream blocks for each 64-byte chunk of ciphertext
        blocks_needed = (len(ciphertext) + 63) // 64

        for block in range(blocks_needed):
            # Generate keystream block with current counter value
            keystream_block = chacha20_block(key, counter_value + block, iv)
            keystream.extend(keystream_block)

        # Trim keystream to match ciphertext length
        keystream = keystream[:len(ciphertext)]

        # XOR keystream with ciphertext to get plaintext
        decrypted_bytes = strxor(ciphertext, keystream)

        # Decode to UTF-8 and parse as JSON
        decrypted_message = decrypted_bytes.decode("utf-8")
        decrypted_payload = json.loads(decrypted_message)

        return decrypted_payload

    except Exception as e:
        logger.error(f"Error decrypting message: {e}")
        return None
```

### MQTT Buffer Size Configuration

The ESP32 device is configured to handle larger encrypted messages by increasing the MQTT buffer size. This is done in the setup function:

```cpp
// Set up MQTT client with larger buffer
if (mqttClient.setBufferSize(1024)) {  // Increase buffer to 1KB
  Serial.println("Buffer size set to 1024 bytes");
} else {
  Serial.println("Failed to set buffer size!");
}
```

The default buffer size in the PubSubClient library is only 256 bytes, which may be too small for encrypted messages with varying lengths. With the buffer increased to 1024 bytes, the device can reliably transmit larger encrypted payloads.

## Contributing

// ...existing code...

## License

[Add your license information here]
