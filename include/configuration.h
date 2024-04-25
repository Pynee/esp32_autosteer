#ifndef CONFIG_H
#define CONFIG_H

#include <WiFiManager.h>
#include "Button.h"
#include "Sensor.h"
#include "IMUHandler.h"

#define DEBUG_PORT 1             // Port for debuging
#define GNSS_PORT 2              // Port for GNSS board
#define AOGNtripPort 2233        // port NTRIP data from AOG comes in
#define AOGAutoSteerPort 8888    // port Autosteer data from AOG comes in
#define DestinationPort 9999     // Port of AOG that listens
#define AOGIP 255, 255, 255, 255 // AOG IP
/*  PWM Frequency ->
     490hz (default)
     122hz
     3921hz
*/
#define PWM_Frequency 490

/////////////////////////////////////////////

// if not in eeprom, overwrite
#define EEP_Ident 2400

//   ***********  Motor drive connections  **************
// Connect ground only for cytron, Connect Ground and +3.3v for IBT2

// Dir1 for Cytron Dir, Both L and R enable for IBT2
#define DIR1_RL_ENABLE_PIN 27

// PWM1 for Cytron PWM, Left PWM for IBT2
#define PWM1_LPWM 13

// Not Connected when using Cytron, Right PWM for IBT2
#define PWM2_RPWM 14

//--------------------------- Switch Input Pins ------------------------
#define STEERSW_PIN 11 // Steerswitch
#define WORKSW_PIN 12  // Workswitch
#define REMOTE_PIN 13  // No idea how this is meant to work. Propably doesn't

// How many degrees before decreasing Max PWM
#define LOW_HIGH_DEGREES 3.0

#define AUTOSTEER_INTERVAL 100 // read GPIO millisec 10hz

// loop time variables in milliseconds
const uint16_t LOOP_TIME = 20; // 50Hz
const uint16_t WATCHDOG_THRESHOLD = 100;

// Define sensor pin for current or pressure sensor
#define LOAD_SENSOR_PIN 17
#define WAS_SENSOR_PIN 18

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
    char imuHeading[6];
    char imuRoll[6];
    char imuPitch[6];
    char imuYawRate[6];
};
extern AOGDataToSend dataToSend;
struct AOGReceivedData // Data struct for variables that are received to AOG
{
    uint8_t guidanceStatusByte; // Byte including all AOG status bits
    double steerTargetAngle;
    float highLowPerDeg; // How many degrees before decreasing Max PWM
    float gpsSpeed;      // GPS Speed in kph
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

struct QueueItem
{
    uint8_t *data;
    uint8_t length;
};
const bool invertRoll = true;                                 // Used for IMU with dual antenna
const uint16_t WATCHDOG_FORCE_VALUE = WATCHDOG_THRESHOLD + 2; // Should be greater than WATCHDOG_THRESHOLD

#endif