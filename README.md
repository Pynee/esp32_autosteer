# AGOpenGPS Autosteer - ESP32 PlatformIO Project

This repository contains a project for implementing **AGOpenGPS Autosteer** functionality using an **ESP32** microcontroller, built and managed using **PlatformIO**. The goal is to integrate AGOpenGPS software with an ESP32 board for autonomous steering in agricultural equipment, enabling precise control of tractors or other vehicles.

The project leverages the **ESP32**'s capabilities, such as Wi-Fi, SPI, Serial, I2C and I/O control, to interface with AGOpenGPS and automate steering systems.

## Features
- **AGOpenGPS integration** for precision steering
- **ESP32** microcontroller with **Wi-Fi** connectivity
- Real-time communication with AGOpenGPS for steering data and commands
- Support for various sensors and actuators for controlling the steering system
- Built and managed using **PlatformIO** for easy deployment and configuration

## Prerequisites
Before you begin, ensure you have the following:

- **ESP32 Development Board** (such as ESP32 DevKitC or any other supported variant)
- **PlatformIO IDE** (recommended) or **PlatformIO Core (CLI)**
- Basic knowledge of **AGOpenGPS** and its requirements for autosteer operation
- Necessary hardware components (e.g., steering motor controller, GPS, sensors)

## Setup

### 1. Install PlatformIO
You will need PlatformIO to manage the build system and upload the code to the ESP32. You can install PlatformIO in two ways:

- **VS Code (Recommended)**:
  1. Install [VS Code](https://code.visualstudio.com/) (if not already installed).
  2. Open **VS Code**, go to the **Extensions** tab, search for **PlatformIO IDE**, and click **Install**.

- **PlatformIO Core (CLI)**:
  Install PlatformIO Core using the following command:
  ```bash
  pip install platformio
  ```

### 2. Clone the Repository
Clone this repository to your local machine:

```bash
git clone --recurse-submodules https://github.com/Pynee/esp32_autosteer.git
cd agopengps-autosteer-esp32
```

### 3. Install Dependencies
PlatformIO will automatically install required libraries and toolchains, but you can manually trigger the installation by running:

```bash
platformio lib install
```

### 4. Configure the `platformio.ini`
Ensure your ESP32 board and settings are correctly configured. Open the `platformio.ini` file and select the correct board for your project:

```ini
[env:esp32-devkitc]
platform = espressif32
board = esp32-devkitc
framework = arduino
monitor_speed = 115200
```

Change the `board` setting if you are using a different ESP32 model. You can find all available boards on [PlatformIO's boards registry](https://platformio.org/boards).

### 5. Upload the Code
Once you’ve configured the project, compile and upload the code to your ESP32:

- **Using PlatformIO IDE**:
  - Press the **Build** button (check mark icon) to compile the project.
  - Press the **Upload** button (arrow icon) to upload the firmware to the ESP32.

- **Using PlatformIO CLI**:
  - Build the project:
    ```bash
    platformio run
    ```
  - Upload the firmware:
    ```bash
    platformio run --target upload
    ```

### 6. Monitor Serial Output
To monitor the serial output (e.g., for debugging or logging), you can use the **Serial Monitor**:

- **Using PlatformIO IDE**:
  - Press the **Serial Monitor** button (plug icon).

- **Using PlatformIO CLI**:
  - Open the serial monitor with the following command:
    ```bash
    platformio device monitor
    ```

## Project Structure

```
.
├── include/               # Header files
├── lib/                   # Libraries
├── src/                   # Source files
│   ├── main.cpp           # Main program entry point for AGOpenGPS autosteer functionality
├── platformio.ini         # PlatformIO configuration file
└── README.md              # Project documentation (this file)
```

## Troubleshooting

- **Board not detected**: Ensure your ESP32 is correctly connected to your computer and that you've selected the correct port in PlatformIO or the `platformio.ini` file.
  
- **Build errors**: Make sure you have all the required libraries installed for handling GPS, motor control, and communication with AGOpenGPS. You can add any missing libraries via PlatformIO.

- **Upload issues**: If you encounter upload issues, try manually pressing the **boot** button on the ESP32 during the upload process to put it into bootloader mode.

- **Serial Output**: If no serial output appears, check the serial baud rate and ensure that the correct COM port is selected in the serial monitor.

## Additional Resources

- [AGOpenGPS Documentation](https://github.com/farmerbriantee/AGOpenGPS)
- [ESP32 Platform Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/)
- [PlatformIO Documentation](https://platformio.org/docs)

## License
This project is licensed under the GPL-3.0 license - see the [LICENSE](LICENSE) file for details.

---



|Task            |trigger/interval                       |priority|purpose          |
|----------------|---------------------------------------|--------|-----------------|
|autosteerWorker |AUTOSTEER_INTERVAL (default 100ms)     |3       |Handles everything steeringmotor/hydraulic related things|
|gnssStreamWorker|trigger on new data from gnss board    |3       |Handles communication to/from gps board|
|imuWorker       |triggers on new data from IMU board    |3       |handles communication from IMU board|
|inputWorker     |LOOP_TIME(default 20ms)|3|Reads switch states(steer and work switch etc.) and analog sensors(WAS for now)|
|uartEventWorker |trigger on new data from UART          |3       |Handles UART(Serial) communication|
|sendDataTask    |trigger when new message added to queue|3       |Handles UDP send queue|
|WifiManager     |???                                    |?       |Handles wifi connection|





Input


using these submodules that you can find in the lib folder:

[WiFiManager](https://github.com/tzapu/WiFiManager)

[Adafruit_BNO08x](https://github.com/adafruit/Adafruit_BNO08x)

[Adafruit_BusIO](https://github.com/adafruit/Adafruit_BusIO.git)

[Adafruit_Sensor](https://github.com/adafruit/Adafruit_Sensor.git)

[AutoPID](https://github.com/r-downing/AutoPID.git)

[SimpleKalmanFilter](https://github.com/denyssene/SimpleKalmanFilter)

