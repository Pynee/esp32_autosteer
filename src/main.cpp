#include <Arduino.h>
#include <EEPROM.h>
#include <WiFiManager.h>
#include "configuration.h"
#include "AutoSteer.h"
#include "GnssHandler.h"
#include "IMUHandler.h"
#include "InputHandler.h"
#include "UDPPacketManager.h"
#include "PandaBuilder.h"
#include "UARTHandler.h"
#include "CommandHandler.h"

// #include "zPackets.cpp"
UART uart1(DEBUG_PORT, commandHandler);

UDPPacketManager packetManager(&gnssSendData);
IMUHandler imuHandler;
PandaBuilder pandaBuilder(&packetManager, &imuHandler);

// GnssHandler gnssHandler;

// Scheduler ts;

// void imuTask();

// void gpsStream();

// void inputHandler();

// void autosteerLoop();

// void commandHandler();

// void autoSteerPacketPerser(AsyncUDPPacket udpPacket);

// Task imuTS(TASK_IMMEDIATE, TASK_FOREVER, &imuTask, &ts, false);

// Task t2(TASK_IMMEDIATE, TASK_FOREVER, &gpsStream, &ts, true);

// Task t3(LOOP_TIME, TASK_FOREVER, &inputHandler, &ts, true);

// Task t4(AUTOSTEER_INTERVAL, TASK_FOREVER, &autosteerLoop, &ts, true);

// Task t5(1000, TASK_FOREVER, &commandHandler, &ts, true);

// EEPROM
int16_t EEread = 0;

void setup()
{

  // Setup Serial Monitor
  // Serial.begin(115200);
  //Serial2.begin(115200);
  packetManager.initUDP();
  imuHandler.initIMU();
  initGnssHandler(&pandaBuilder);
  initInput();
  initAutosteer();
}

void loop()
{
  // ts.execute();
}