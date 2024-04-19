#ifndef IMUHANDLER_H
#define IMUHANDLER_H

#include <Adafruit_BNO08x.h>
#include "configuration.h"

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
    sh2_SensorValue_t sensorValue;
    Adafruit_BNO08x bno08x;
    const int16_t nrBNO08xAdresses = sizeof(bno08xAddresses) / sizeof(bno08xAddresses[0]);
    uint8_t bno08xAddress;
    void saveQuaternion(float qr, float qi, float qj, float qk, IMUVector *imuVector);
    void quaternionToEuler(float qr, float qi, float qj, float qk, euler_t *ypr);
    static void startWorkerImpl(void *);
    void imuWorker(void *z);
    void setReports();
    uint8_t error;
    euler_t ypr;
    IMUVector imuVector;

public:
    IMUHandler();
    void initIMU();
    void calculateEuler();
    char imuHeading[6];
    char imuRoll[6];
    char imuPitch[6];
    char imuYawRate[6];
    // booleans to see if we are using BNO08x
    bool useBNO08x = false;
};

#endif