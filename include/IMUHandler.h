#ifndef IMUHANDLER_H
#define IMUHANDLER_H

#include <Adafruit_BNO08x.h>
#include "configuration.h"
#include "GlobalVariables.h"

#define BNO08X_RESET 5

// For SPI mode, we need a CS pin
#define BNO08X_CS 10
#define BNO08X_INT 9

#define RAD_TO_DEG_X_10 572.95779513082320876798154814105

struct IMUVector
{
    uint32_t time;
    float qr;
    float qi;
    float qj;
    float qk;
};

struct euler_t
{
    float yaw;
    float pitch;
    float roll;
};
// BNO08x address variables to check where it is
const uint8_t bno08xAddresses[] = {0x4A, 0x4B};

// IMU

class IMUHandler
{
private:
    void quaternionToEuler(IMUVector *imuVector, euler_t *rotationAxes);
    void saveToGlobal(euler_t *rotationAxes);
    static void startWorkerImpl(void *_this);
    void imuWorker(void *z);
    void setReports();
    euler_t rotationAxes;
    IMUVector imuVector;

public:
    IMUHandler();
    void init();
    void calculateEuler();
};

#endif