#include "IMUHandler.h"

IMUHandler::IMUHandler()
{
}

void IMUHandler::initIMU()
{
    // set up communication
    Wire.begin();
    for (int16_t i = 0; i < nrBNO08xAdresses; i++)
    {
        bno08xAddress = bno08xAddresses[i];

        // Serial.print("\r\nChecking for BNO08X on ");
        // Serial.println(bno08xAddress, HEX);
        Wire.beginTransmission(bno08xAddress);
        error = Wire.endTransmission();

        if (error == 0)
        {
            // Serial.println("Error = 0");
            Serial.print("0x");
            Serial.print(bno08xAddress, HEX);
            Serial.println(" BNO08X Ok.");
            // Initialize BNO080 lib
            if (bno08x.begin_I2C((int32_t)bno08xAddress)) //??? Passing NULL to non pointer argument, remove maybe ???
            {
                setReports();
                useBNO08x = true;
            }
            else
            {
                Serial.println("BNO080 not detected at given I2C address.");
            }
        }
        else
        {
            // Serial.println("Error = 4");
            Serial.print("0x");
            Serial.print(bno08xAddress, HEX);
            Serial.println(" BNO08X not Connected or Found");
        }
        if (useBNO08x)
        {
            // imuTS.enable();
            xTaskCreate(this->startWorkerImpl, "imuWorker", 3096, NULL, 3, NULL);
            break;
        }
    }
}
void IMUHandler::startWorkerImpl(void *_this)
{
    ((IMUHandler *)_this)->imuWorker(_this);
}

void IMUHandler::imuWorker(void *z)
{
    for (;;)
    {
        if (!useBNO08x)
        {
            return;
        }
        if (bno08x.wasReset())
        {
            Serial.print("sensor was reset ");
            setReports();
        }

        if (!bno08x.getSensorEvent(&sensorValue))
        {
            return;
        }

        switch (sensorValue.sensorId)
        {

        case SH2_GAME_ROTATION_VECTOR:
            saveQuaternion(sensorValue.un.gameRotationVector.real, sensorValue.un.gameRotationVector.i, sensorValue.un.gameRotationVector.j, sensorValue.un.gameRotationVector.k, &imuVector);
            break;
        }
    }
}

void IMUHandler::saveQuaternion(float qr, float qi, float qj, float qk, IMUVector *imuVector)
{
    imuVector->time = millis();
    imuVector->qr = qr;
    imuVector->qi = qi;
    imuVector->qj = qj;
    imuVector->qk = qk;
}

void IMUHandler::calculateEuler()
{
    quaternionToEuler(imuVector.qr, imuVector.qi, imuVector.qj, imuVector.qk, &ypr);
    // int16_t temp = 0;

    // BNO is reading in its own timer
    //  Fill rest of Panda Sentence - Heading
    // temp = ypr.yaw;
    itoa(ypr.yaw, imuHeading, 10);

    // the pitch x10
    // temp = (int16_t)ypr.pitch;
    itoa((int16_t)ypr.pitch, imuPitch, 10);

    // the roll x10
    // temp = (int16_t)ypr.roll;
    itoa((int16_t)ypr.roll, imuRoll, 10);

    // YawRate - 0 for now
    itoa(0, imuYawRate, 10);
}

void IMUHandler::quaternionToEuler(float qr, float qi, float qj, float qk, euler_t *ypr)
{

    float sqr = sq(qr);
    float sqi = sq(qi);
    float sqj = sq(qj);
    float sqk = sq(qk);

    ypr->yaw = atan2(2.0 * (qi * qj + qk * qr), (sqi - sqj - sqk + sqr));
    if (steerConfig.IsUseY_Axis)
    {
        ypr->pitch = asin(-2.0 * (qi * qk - qj * qr) / (sqi + sqj + sqk + sqr));
        ypr->roll = atan2(2.0 * (qj * qk + qi * qr), (-sqi - sqj + sqk + sqr));
    }
    else
    {
        ypr->roll = asin(-2.0 * (qi * qk - qj * qr) / (sqi + sqj + sqk + sqr));
        ypr->pitch = atan2(2.0 * (qj * qk + qi * qr), (-sqi - sqj + sqk + sqr));
    }

    ypr->yaw *= -RAD_TO_DEG_X_10;
    if (ypr->yaw < 0)
    {
        ypr->yaw += 3600;
    }
    ypr->pitch *= RAD_TO_DEG_X_10;
    ypr->roll *= RAD_TO_DEG_X_10;

    if (invertRoll)
    {
        ypr->roll *= -1;
    }
}

void IMUHandler::setReports()
{
    if (!bno08x.enableReport(SH2_GAME_ROTATION_VECTOR, 20000))
    {
        Serial.println("Could not enable stabilized remote vector");
        return;
    }
}
