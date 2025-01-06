This is originally a fork of MrPoke21's repo https://github.com/MrPoke21/AgOpenGPS_Boards/tree/master/ESP32 that was converted into Platformio project but now almost everything is totally rewritten. My goal is to make a better codebase for AgOpenGPS firmware targeting ESP32 boards. 

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


using these submodules tha you can find in the lib folder:
https://github.com/tzapu/WiFiManager

https://github.com/pfeerick/elapsedMillis

https://github.com/adafruit/Adafruit_BNO08x
https://github.com/adafruit/Adafruit_BusIO.git
https://github.com/adafruit/Adafruit_Sensor.git

https://github.com/r-downing/AutoPID.git

https://github.com/denyssene/SimpleKalmanFilter
