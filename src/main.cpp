#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"
#include <Arduino.h>
// #include "UARTHandler.h"
#include "configuration.h"
#include "CommandHandler.h"

// #include <WiFiManager.h>

#include "Autosteerhandler.h"
#include "GnssHandler.h"
#include "IMUHandler.h"
#include "InputHandler.h"
#include "UDPPacketManager.h"
#include "PandaBuilder.h"
#include "PGNCommManager.h"

GNSSHandler gnssHandler;
UDPPacketManager packetManager(&gnssHandler);
IMUHandler imuHandler;
PGNCommManager pgnCommManager(&packetManager);
PandaBuilder pandaBuilder(&pgnCommManager);
AutosteerHandler autosteerHandler;

// GnssHandler gnssHandler;

// initializing Global variables;
int count = 0;
IPAddress deviceIP;
IPAddress destinationIP(AOGIP);
bool destinationIPSet = false;
Setup steerConfig;
SteerSettings steerSettings;
Sensors sensors;
AOGDataToSend dataToSend;
AOGReceivedData receivedData;
HardwareSwitches hardwareSwitches;
State state;
//-----------------------------------

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.setDebugOutput(true);
  Serial.begin(115200);
  packetManager.init(&pgnCommManager);
  pgnCommManager.init();
  imuHandler.init();
  gnssHandler.init(&pandaBuilder);
  initInput();
  autosteerHandler.init();
}

void loop()
{
  // char stats[1024];
  if (count++ > 1000000)
  {
    // vTaskGetRunTimeStats(stats);
    // Serial.write(stats, sizeof(stats));
    count = 0;
  }
}