#ifndef GLOBALVARIABLES_H
#define GLOBALVARIABLES_H

#include <string>
#include <Arduino.h>
#include <ESP.h>
#include <esp_wifi.h>
#include <elapsedMillis.h>
#include <WiFiManager.h>
#include "AsyncUDP.h"
#include <EEPROM.h>
#include "configuration.h"
// #include "UARTHandler.h"
#include "PGNParser.h"
#include "queueItem.h"
#include "Button.h"
#include "Sensor.h"
#include "IMUHandler.h"

extern IPAddress deviceIP;
extern IPAddress destinationIP;
extern bool destinationIPSet;
struct SteerSettings
{
    float Kp = 40; // proportional gain
    float Kd = 20;
    float Ki = 0.5;
    uint8_t lowPWM = 10; // band of no action
    int16_t wasOffset = 0;
    uint8_t minPWM = 9;
    uint8_t highPWM = 60;         // max PWM value
    float steerSensorCounts = 30; // How many steps in sensor is one degree in angle
    float AckermanFix = 1;        // sent as percent
};
extern SteerSettings steerSettings; // 11 bytes

// Variables for settings - 0 is false
struct Setup
{
    uint8_t InvertWAS = 0;
    uint8_t IsRelayActiveHigh = 0; // if zero, active low (default)
    uint8_t MotorDriveDirection = 0;
    uint8_t SingleInputWAS = 1;
    uint8_t CytronDriver = 1;
    uint8_t SteerSwitch = 0; // 1 if switch selected
    uint8_t SteerButton = 0; // 1 if button selected
    uint8_t ShaftEncoder = 0;
    uint8_t PressureSensor = 0;
    uint8_t CurrentSensor = 0;
    uint8_t PulseCountMax = 5; // if encoder PulseCountMax else maxcurrentpercent/maxpressurepercent
    uint8_t IsDanfoss = 0;
    uint8_t IsUseY_Axis = 0; // Set to 0 to use X Axis, 1 to use Y avis
};
extern Setup steerConfig; // 9 bytes

struct Sensors
{
    Sensor wheelAngleSensor = Sensor(WAS_SENSOR_PIN, 5, 5, 00.1);
    Sensor loadSensor = Sensor(LOAD_SENSOR_PIN, 2, 2, 0.01);
};
extern Sensors sensors;

// GLOBAL VARIABLES

struct AOGDataToSend // Data struct for variables that are sent to AOG
{
    uint16_t helloSteerPosition;
    double currentSteerAngle;
    uint8_t pwmDisplay;
    bool imuAvailable;
    uint8_t imuHeading[6];
    int16_t heading;
    uint8_t imuRoll[6];
    int16_t roll;
    uint8_t imuPitch[6];
    int16_t pitch;
    uint8_t imuYawRate[6];
    int16_t yawRate;
};
extern AOGDataToSend dataToSend;
struct AOGReceivedData // Data struct for variables that are received to AOG
{
    uint8_t guidanceStatusByte; // Byte including all AOG status bits
    double steerTargetAngle;
    float highLowPerDeg;        // How many degrees before decreasing Max PWM
    float gpsSpeed;             // GPS Speed in kph
    uint16_t sectionRelayBytes; // bytes containing section relay states
    uint8_t tram;               // ???no idea what this is. Distance from the guidance line?
};
extern AOGReceivedData receivedData;

struct HardwareSwitches
{
    Button remoteSwitch = Button(REMOTE_PIN);
    Button workSwitch = Button(WORKSW_PIN);
    Button steerSwitch = Button(STEERSW_PIN);
    uint8_t switchByte = 0;
};
extern HardwareSwitches hardwareSwitches;

struct State
{
    bool autoSteerEnabled = false;
    bool guidanceStatus = false;
    unsigned long lastAOGUpdate;
};
extern State state;

struct NMEAMessage
{
    char fixTime[12];
    char latitude[15];
    char latNS[3];
    char longitude[15];
    char lonEW[3];
    char fixQuality[2];
    char numSats[4];
    char HDOP[5];
    char altitude[12];
    char ageDGPS[10];

    // VTG
    char vtgHeading[12] = {};
    char speedKnots[10] = {};
};

/*struct QueueItem
{
    uint8_t *data;
    uint8_t length;
};*/
const bool invertRoll = true;                                 // Used for IMU with dual antenna
const uint16_t WATCHDOG_FORCE_VALUE = WATCHDOG_THRESHOLD + 2; // Should be greater than WATCHDOG_THRESHOLD

#endif