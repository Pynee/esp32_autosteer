#ifndef CONFIG_H
#define CONFIG_H

#include <WiFiManager.h>
#include "Button.h"
#include "Sensor.h"
#define AOGNtripPort 2233        // port NTRIP data from AOG comes in
#define AOGAutoSteerPort 8888    // port Autosteer data from AOG comes in
#define DestinationPort 9999     // Port of AOG that listens
#define AOGIP 255, 255, 255, 255 // AOG IP
/*  PWM Frequency ->
     490hz (default) = 0
     122hz = 1
     3921hz = 2
*/
#define PWM_Frequency 0

/////////////////////////////////////////////

// if not in eeprom, overwrite
#define EEP_Ident 2400

//   ***********  Motor drive connections  **************888
// Connect ground only for cytron, Connect Ground and +5v for IBT2

// Dir1 for Cytron Dir, Both L and R enable for IBT2
#define DIR1_RL_ENABLE_PIN 27

// PWM1 for Cytron PWM, Left PWM for IBT2
#define PWM1_LPWM 13

// Not Connected for Cytron, Right PWM for IBT2
#define PWM2_RPWM 14

//--------------------------- Switch Input Pins ------------------------
#define STEERSW_PIN 25
#define WORKSW_PIN 26
#define REMOTE_PIN 33 // No idea how this is meant to work. Propably doesn't

#define CONST_180_DIVIDED_BY_PI 57.2957795130823

// How many degrees before decreasing Max PWM
#define LOW_HIGH_DEGREES 3.0

#define AUTOSTEER_INTERVAL 100 // read GPIO millisec 10hz

// loop time variables in milliseconds
const uint16_t LOOP_TIME = 20; // 50Hz
const uint16_t WATCHDOG_THRESHOLD = 100;

// Define sensor pin for current or pressure sensor
#define LOAD_SENSOR_PIN 39
#define WAS_SENSOR_PIN 36

struct SteerSettings
{
    uint8_t Kp = 40;     // proportional gain
    uint8_t lowPWM = 10; // band of no action
    int16_t wasOffset = 0;
    uint8_t minPWM = 9;
    uint8_t highPWM = 60;         // max PWM value
    float steerSensorCounts = 30; // How many steps in sensor is one degree in angle
    float AckermanFix = 1;        // sent as percent
};
static SteerSettings steerSettings; // 11 bytes

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
static Setup steerConfig; // 9 bytes

struct Sensors
{
    Sensor wheelAngleSensor = Sensor(WAS_SENSOR_PIN, 5, 5, 00.1);
    Sensor loadSensor = Sensor(LOAD_SENSOR_PIN, 2, 2, 0.01);
};
static Sensors sensors;

// GLOBAL VARIABLES

struct AOGDataToSend // Data struct for variables that are sent to AOG
{
    uint16_t helloSteerPosition;
    float currentSteerAngle;
    uint8_t pwmDisplay;
};
static AOGDataToSend dataToSend;
struct AOGReceivedData // Data struct for variables that are received to AOG
{
    uint8_t guidanceStatusByte; // Byte including all AOG status bits
    float steerTargetAngle;
    float highLowPerDeg; // How many degrees before decreasing Max PWM
    float gpsSpeed;      // GPS Speed in kph
};
static AOGReceivedData ReceivedData;

struct HardwareSwitches
{
    Button remoteSwitch = Button(REMOTE_PIN);
    Button workSwitch = Button(WORKSW_PIN);
    Button steerSwitch = Button(STEERSW_PIN);
    uint8_t switchByte = 0;
};
static HardwareSwitches hardwareSwitches;

struct State
{
    bool autoSteerEnabled = false;
    bool guidanceStatus = false;
};
static State state;

void (*sendDatafn)(uint8_t *, uint8_t);
// Create WiFiManager object
WiFiManager wfm;

// speed sent as *10
float gpsSpeed = 0;
bool GGA_Available = false; // Do we have GGA on correct port?

const bool invertRoll = true; // Used for IMU with dual antenna

// On Off
bool guidanceStatusChanged = false;
uint8_t guidanceStatus = 0;
uint8_t prevGuidanceStatus = 0;

float sensorReading;

// booleans to see if we are using BNO08x
bool useBNO08x = false;
uint8_t error;

// steering variables
float steerAngleSetPoint = 0; // the desired angle from AgOpen
int16_t steeringPosition = 0; // from steering sensor
float steerAngleError = 0;    // setpoint - actual
int16_t helloSteerPosition = 0;
float rawSensor, rawWAS;

// Hardware Switches
// uint8_t remoteSwitch = 0, workSwitch = 0, steerSwitch = 1,
uint8_t switchByte = 0;

uint8_t currentState = 1, reading, previous = 0;

const uint16_t WATCHDOG_FORCE_VALUE = WATCHDOG_THRESHOLD + 2; // Should be greater than WATCHDOG_THRESHOLD
uint8_t watchdogTimer = WATCHDOG_FORCE_VALUE;

#endif