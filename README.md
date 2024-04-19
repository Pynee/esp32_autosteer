This Orginaly a fork of this repo that was converted into Platformio project
https://github.com/MrPoke21/AgOpenGPS_Boards/tree/master/ESP32 but now is pretty much totally rewritten. I'm trying to make a more cleanly written codebase for AgOpenGPS firmware targeting ESP32 boards. 

Tasks:
autosteerWorker     //Handles everything steeringmotor/hydraulic related things
gnssStreamWorker    //Handles communication to/from gps board
imuWorker           //handles communication from IMU board
inputWorker         //Reads switch states(steer and work switch etc.) and analog sensors(WAS for now)
uartEventWorker     //Handles UART(Serial) communication
sendDataTask        //Handles UDP send queue 
WifiManager         //Handles wifi connection 





Input


using these submodules in lib folder:
https://github.com/tzapu/WiFiManager

https://github.com/pfeerick/elapsedMillis

https://github.com/adafruit/Adafruit_BNO08x
https://github.com/adafruit/Adafruit_BusIO.git
https://github.com/adafruit/Adafruit_Sensor.git

https://github.com/r-downing/AutoPID.git

https://github.com/denyssene/SimpleKalmanFilter
