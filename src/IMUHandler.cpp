#include "IMUHandler.h"
#include <stdio.h>
#include "BNO08x.hpp"

IMUHandler::IMUHandler()
{
}

void IMUHandler::init()
{
    xTaskCreate(this->startWorkerImpl, "imuWorker", 8192, this, 3, NULL);
}

void IMUHandler::startWorkerImpl(void *_this)
{

    // static_cast<IMUHandler *>(_this)->imuWorker();
    ((IMUHandler *)_this)->imuWorker(&_this);
}

void IMUHandler::imuWorker(void *z)
{
    bool useBNO08x = false;
    Adafruit_BNO08x bno08x(BNO08X_RESET);
    sh2_SensorValue_t sensorValue;
    if (!bno08x.begin_SPI(BNO08X_CS, BNO08X_INT))
    {
        Serial.println("Failed to find BNO08x chip");
    }
    else
    {
        Serial.println("BNO08x Found!");
        dataToSend.imuAvailable = true;
        useBNO08x = true;
        /*
        if (!bno08x.enableReport(SH2_GAME_ROTATION_VECTOR, 20000))
        {
            Serial.println("Could not enable stabilized remote vector");
            return;
        }
        */
        if (!bno08x.enableReport(SH2_ROTATION_VECTOR, 20000))
        {
            Serial.println("Could not enable rotation vector");
        }
        TickType_t xLastWakeTime = xTaskGetTickCount();
        for (;;)
        {
            vTaskDelayUntil(&xLastWakeTime, 20 / portTICK_PERIOD_MS);

            if (bno08x.wasReset())
            {
                Serial.print("sensor was reset ");
                /*
                if (!bno08x.enableReport(SH2_GAME_ROTATION_VECTOR))
                {
                    Serial.println("Could not enable game vector");
                }
                */
                if (!bno08x.enableReport(SH2_ROTATION_VECTOR))
                {
                    Serial.println("Could not enable rotation vector");
                }
            }

            if (bno08x.getSensorEvent(&sensorValue))
            {
                switch (sensorValue.sensorId)
                {
                /*
                case SH2_GAME_ROTATION_VECTOR:
                {
                    imuVector.time = millis();
                    imuVector.qr = sensorValue.un.gameRotationVector.real;
                    imuVector.qi = sensorValue.un.gameRotationVector.i;
                    imuVector.qj = sensorValue.un.gameRotationVector.j;
                    imuVector.qk = sensorValue.un.gameRotationVector.k;

                    break;
                }
                */
                case SH2_ROTATION_VECTOR:
                {
                    imuVector.time = millis();
                    imuVector.qr = sensorValue.un.rotationVector.real;
                    imuVector.qi = sensorValue.un.rotationVector.i;
                    imuVector.qj = sensorValue.un.rotationVector.j;
                    imuVector.qk = sensorValue.un.rotationVector.k;
                    calculateEuler();
                    break;
                }
                }
            }
        }
    }
    vTaskDelete(NULL);
}

void IMUHandler::calculateEuler()
{
    quaternionToEuler(&imuVector, &rotationAxes);
    saveToGlobal(&rotationAxes);
    // int16_t temp = 0;
}

void IMUHandler::quaternionToEuler(IMUVector *imuVector, euler_t *rotationAxes)
{

    float sqr = sq(imuVector->qr);
    float sqi = sq(imuVector->qi);
    float sqj = sq(imuVector->qj);
    float sqk = sq(imuVector->qk);

    rotationAxes->yaw = atan2(2.0 * (imuVector->qi * imuVector->qj + imuVector->qk * imuVector->qr), (sqi - sqj - sqk + sqr)) * -RAD_TO_DEG;
    if (rotationAxes->yaw < 0)
    {
        rotationAxes->yaw += 360;
    }
    if (steerConfig.IsUseY_Axis)
    {
        rotationAxes->pitch = asin(-2.0 * (imuVector->qi * imuVector->qk - imuVector->qj * imuVector->qr) / (sqi + sqj + sqk + sqr)) * RAD_TO_DEG;
        rotationAxes->roll = atan2(2.0 * (imuVector->qj * imuVector->qk + imuVector->qi * imuVector->qr), (-sqi - sqj + sqk + sqr)) * RAD_TO_DEG;
    }
    else
    {
        rotationAxes->roll = asin(-2.0 * (imuVector->qi * imuVector->qk - imuVector->qj * imuVector->qr) / (sqi + sqj + sqk + sqr)) * RAD_TO_DEG;
        rotationAxes->pitch = atan2(2.0 * (imuVector->qj * imuVector->qk + imuVector->qi * imuVector->qr), (-sqi - sqj + sqk + sqr)) * RAD_TO_DEG;
    }
    if (invertRoll)
    {
        rotationAxes->roll *= -1;
    }

    /*
       ypr->yaw *= -RAD_TO_DEG_X_10;
       if (ypr->yaw < 0)
       {
           ypr->yaw += 3600;
       }
       ypr->pitch *= RAD_TO_DEG_X_10;
       ypr->roll *= RAD_TO_DEG_X_10;


       */
}

void IMUHandler::saveToGlobal(euler_t *rotationAxes)
{
    dataToSend.heading = rotationAxes->yaw * 10;
    dataToSend.pitch = rotationAxes->pitch * 10;
    dataToSend.roll = rotationAxes->roll * 10;
    dataToSend.yawRate = 0;
}
