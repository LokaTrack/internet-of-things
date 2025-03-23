# LokaTrack Internet of Things

This repository hosts the source code for the LokaTrack IoT project. This project is designed to track the location of a driver using a GPS module and send the data to a server using a GSM module.

## Table of Contents

- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Software Requirements](#software-requirements)
- [Installation](#installation)
- [Usage](#usage)
- [Contributing](#contributing)
- [License](#license)

## Hardware Requirements

- ESP32 development board.
- NEO-6M GPS module.
- SIM800L GSM module.
- Micro USB cable for programming and power
- Jumper wires.
- Breadboard (for prototyping) or PCB (for final product).
- Antennas for both GPS and GSM modules

## Software Requirements

- [PlatformIO IDE](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide) with [pioarduino extension](https://marketplace.visualstudio.com/items?itemName=pioarduino.pioarduino-ide) or Arduino IDE.
- [Espressif ESP32 Dev Module](https://docs.platformio.org/en/latest/boards/espressif32/esp32dev.html?utm_source=platformio&utm_medium=piohome) board support package.
- Libraries:
  - [TinyGPS++](https://github.com/mikalhart/TinyGPSPlus?utm_source=platformio&utm_medium=piohome) (for GPS data parsing).
  - [TinyGSM](https://github.com/vshymanskyy/TinyGSM?utm_source=platformio&utm_medium=piohome) (for GSM communication).
  - [ArduinoJSON](https://github.com/bblanchon/ArduinoJson.git?utm_source=platformio&utm_medium=piohome) (for JSON data handling).

## Installation

## Usage

## Contributing

## License
