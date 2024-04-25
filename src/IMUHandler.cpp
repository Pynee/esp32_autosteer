#include "IMUHandler.h"

IMUHandler::IMUHandler()
{
}

void IMUHandler::initIMU()
{
    Serial.begin(115200);

    Serial.println("Adafruit BNO08x test!");

    // Try to initialize!
    if (!bno08x.begin_I2C())
    {
        Serial.println("Failed to find BNO08x chip");
    }
    else
    {
        Serial.println("BNO08x Found!");
        useBNO08x = true;

        for (int n = 0; n < bno08x.prodIds.numEntries; n++)
        {
            Serial.print("Part ");
            Serial.print(bno08x.prodIds.entry[n].swPartNumber);
            Serial.print(": Version :");
            Serial.print(bno08x.prodIds.entry[n].swVersionMajor);
            Serial.print(".");
            Serial.print(bno08x.prodIds.entry[n].swVersionMinor);
            Serial.print(".");
            Serial.print(bno08x.prodIds.entry[n].swVersionPatch);
            Serial.print(" Build ");
            Serial.println(bno08x.prodIds.entry[n].swBuildNumber);
        }

        setReports();

        Serial.println("Reading events");
        delay(100);
        /*Serial.println("Adafruit BNO08x test!");

        // Try to initialize!
        if (!bno08x.begin_I2C())
        {
            Serial.println("Failed to find BNO08x chip");
        }
        else
        {
            Serial.println("BNO08x Found!");
            Serial.println("Reading events");

     setReports();*/

        // imuTS.enable();

        if (!bno08x.getSensorEvent(&sensorValue))
        {
            return;
        }
        else
        {
            switch (sensorValue.sensorId)
            {

            case SH2_GAME_ROTATION_VECTOR:
                Serial.print("Game Rotation Vector - r: ");
                Serial.print(sensorValue.un.gameRotationVector.real);
                Serial.print(" i: ");
                Serial.print(sensorValue.un.gameRotationVector.i);
                Serial.print(" j: ");
                Serial.print(sensorValue.un.gameRotationVector.j);
                Serial.print(" k: ");
                Serial.println(sensorValue.un.gameRotationVector.k);
                break;
            }
        }
    }

    xTaskCreate(this->startWorkerImpl, "imuWorker", 8192, NULL, 3, NULL);
}

void IMUHandler::startWorkerImpl(void *_this)
{
    ((IMUHandler *)_this)->imuWorker(_this);
}

void IMUHandler::imuWorker(void *z)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    for (;;)
    {
        /*
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

        */
        vTaskDelayUntil(&xLastWakeTime, 20 / portTICK_PERIOD_MS);
        delay(10);

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
            Serial.print("Game Rotation Vector - r: ");
            Serial.print(sensorValue.un.gameRotationVector.real);
            Serial.print(" i: ");
            Serial.print(sensorValue.un.gameRotationVector.i);
            Serial.print(" j: ");
            Serial.print(sensorValue.un.gameRotationVector.j);
            Serial.print(" k: ");
            Serial.println(sensorValue.un.gameRotationVector.k);
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
