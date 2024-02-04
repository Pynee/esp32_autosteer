#include <Arduino.h>
#include <TaskScheduler.h>
#include <elapsedMillis.h>
#include <EEPROM.h>
#include <Wire.h>

#include <WiFiManager.h>
#include "AsyncUDP.h"
#include "AutosteerPID.cpp"
#include "configuration.h"
#include "zAutoSteer.cpp"
#include "zCommands.cpp"
#include "zHandlers.cpp"
#include "zInput.cpp"
#include "zNMEAParser.h"
#include "zPackets.cpp"

Scheduler ts;

// Create WiFiManager object
WiFiManager wfm;

void imuTask();

void gpsStream();

void inputHandler();

void autosteerLoop();

void commandHandler();

void autoSteerPacketPerser(AsyncUDPPacket udpPacket);

Task imuTS(TASK_IMMEDIATE, TASK_FOREVER, &imuTask, &ts, false);

Task t2(TASK_IMMEDIATE, TASK_FOREVER, &gpsStream, &ts, true);

Task t3(LOOP_TIME, TASK_FOREVER, &inputHandler, &ts, true);

Task t4(AUTOSTEER_INTERVAL, TASK_FOREVER, &autosteerLoop, &ts, true);

Task t5(1000, TASK_FOREVER, &commandHandler, &ts, true);

// EEPROM
int16_t EEread = 0;

// On Off
uint8_t guidanceStatus = 0;
uint8_t prevGuidanceStatus = 0;
bool guidanceStatusChanged = false;

// speed sent as *10
float gpsSpeed = 0;
bool GGA_Available = false; // Do we have GGA on correct port?

const bool invertRoll = true; // Used for IMU with dual antenna

// Variables for settings

void steerSettingsInit()
{
  // for PWM High to Low interpolator
  highLowPerDeg = ((float)(steerSettings.highPWM - steerSettings.lowPWM)) / LOW_HIGH_DEGREES;
}

void autosteerSetup()
{
  // PWM rate settings. Set them both the same!!!!
  /*  PWM Frequency ->
       490hz (default) = 0
       122hz = 1
       3921hz = 2
  */
  if (PWM_Frequency == 0)
  {
    ledcSetup(PWM1_LPWM, 490, 8);
    ledcSetup(PWM2_RPWM, 490, 8);
  }
  else if (PWM_Frequency == 1)
  {
    ledcSetup(PWM1_LPWM, 122, 8);
    ledcSetup(PWM2_RPWM, 122, 8);
  }
  else if (PWM_Frequency == 2)
  {
    ledcSetup(PWM1_LPWM, 3921, 8);
    ledcSetup(PWM2_RPWM, 3921, 8);
  }

  pinMode(DIR1_RL_ENABLE, OUTPUT);

  // Disable digital inputs for analog input pins
  // pinMode(CURRENT_SENSOR_PIN, INPUT_DISABLE);
  // pinMode(PRESSURE_SENSOR_PIN, INPUT_DISABLE);

  EEPROM.begin(60);
  EEPROM.get(0, EEread); // read identifier

  if (EEread != EEP_Ident) // check on first start and write EEPROM
  {
    EEPROM.put(0, EEP_Ident);
    EEPROM.put(10, steerSettings);
    EEPROM.put(40, steerConfig);
    EEPROM.commit();
  }
  else
  {
    EEPROM.get(10, steerSettings); // read the Settings
    EEPROM.get(40, steerConfig);
  }

} // End of Setup

void setup()
{
  // Setup Serial Monitor
  Serial.begin(115200);
  Serial2.begin(115200);
  initUDP();
  initHandler();

  initIMU();

  initInput();

  autosteerSetup();
}

void loop()
{
  ts.execute();
}