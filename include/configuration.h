#ifndef CONFIG_H
#define CONFIG_H

#define DEBUG_PORT 1 // Port for debuging
#define GNSS_PORT 2  // Port for GNSS board
#define GNSS_TX_PIN 39
#define GNSS_RX_PIN 38
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
#define DIR1_RL_ENABLE_PIN 8

// PWM1 for Cytron PWM, Left PWM for IBT2
#define PWM1_LPWM 15

// Not Connected when using Cytron, Right PWM for IBT2
#define PWM2_RPWM 14

//--------------------------- Switch Input Pins ------------------------
#define STEERSW_PIN 18 // Steerswitch
#define WORKSW_PIN 17  // Workswitch
#define REMOTE_PIN 16  // No idea how this is meant to work. Propably doesn't

// How many degrees before decreasing Max PWM
#define LOW_HIGH_DEGREES 3.0

#define AUTOSTEER_INTERVAL 100 // read GPIO millisec 10hz
#define WIFIMANAGER_INTERVAL 50
// loop time variables in milliseconds
const uint16_t LOOP_TIME = 20; // 50Hz
const uint16_t WATCHDOG_THRESHOLD = 100;
const uint16_t AOGTIMEOUT = 100; // How many milliseconds without steering updates from AOG before we disable autosteer

// Define sensor pin for current or pressure sensor
#define LOAD_SENSOR_PIN 17
#define WAS_SENSOR_PIN 18

#define useSerial true
#define useWifi true
#define useEthernet true
#define useCan true

uint8_t board = 2;

#endif