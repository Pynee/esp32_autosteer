#include <Arduino.h>
#include "UARTHandler.h"
#include "configuration.h"
#include "CommandHandler.h"
#include "USBSerial.h"

#include <EEPROM.h>
#include <WiFiManager.h>

#include "AutoSteer.h"
#include "GnssHandler.h"
#include "IMUHandler.h"
#include "InputHandler.h"
#include "UDPPacketManager.h"
#include "PandaBuilder.h"

// #include "zPackets.cpp"
UART uart1(DEBUG_PORT, commandHandler);

UDPPacketManager packetManager(&gnssSendData);
// IMUHandler imuHandler;
//  PandaBuilder pandaBuilder(&packetManager, &imuHandler);

// GnssHandler gnssHandler;

// EEPROM
int16_t EEread = 0;
int count = 0;
Setup steerConfig;
SteerSettings steerSettings;
Sensors sensors;
AOGDataToSend dataToSend;
AOGReceivedData receivedData;
HardwareSwitches hardwareSwitches;
State state;

void setup()
{
  // usbSerial.setonReceive(&commandHandler);
  usbOnReceive = commandHandler;
  Serial.begin(115200);
  Serial.onEvent(usbEventCallback);
  Serial0.begin(115200);
  // while (!Serial.available())
  //{
  // };

  packetManager.initUDP();
  // imuHandler.initIMU();
  //  initGnssHandler(&pandaBuilder);
  initInput();
  // initAutosteer();
}

void loop()
{
  // uart1.println("test1");
  // Serial.print("count:");
  // Serial.println(count);
  // Serial0.println(count++);
  // Serial.println("Test");
  //  ts.execute();
}