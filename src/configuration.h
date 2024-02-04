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
#define DIR1_RL_ENABLE 27

// PWM1 for Cytron PWM, Left PWM for IBT2
#define PWM1_LPWM 13

// Not Connected for Cytron, Right PWM for IBT2
#define PWM2_RPWM 14

//--------------------------- Switch Input Pins ------------------------
#define STEERSW_PIN 25
#define WORKSW_PIN 26
#define REMOTE_PIN 33

#define CONST_180_DIVIDED_BY_PI 57.2957795130823

// How many degrees before decreasing Max PWM
#define LOW_HIGH_DEGREES 3.0

#define AUTOSTEER_INTERVAL 100 // read GPIO millisec 50hz

// loop time variables in microseconds
const uint16_t LOOP_TIME = 20; // 40Hz
const uint16_t WATCHDOG_THRESHOLD = 100;

// Define sensor pin for current or pressure sensor
#define LOAD_SENSOR_PIN 39
#define WAS_SENSOR_PIN 36

struct Storage
{
    uint8_t Kp = 40;     // proportional gain
    uint8_t lowPWM = 10; // band of no action
    int16_t wasOffset = 0;
    uint8_t minPWM = 9;
    uint8_t highPWM = 60; // max PWM value
    float steerSensorCounts = 30;
    float AckermanFix = 1; // sent as percent
};
Storage steerSettings; // 11 bytes

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
    uint8_t PulseCountMax = 5;
    uint8_t IsDanfoss = 0;
    uint8_t IsUseY_Axis = 0; // Set to 0 to use X Axis, 1 to use Y avis
};
Setup steerConfig; // 9 bytes