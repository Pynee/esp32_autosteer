#ifndef SENSOR_H
#define SENSOR_H

#include <SimpleKalmanFilter.h>
#include <Arduino.h>

class Sensor
{
private:
    int pin = 0;

    SimpleKalmanFilter sensorFilter;

public:
    Sensor(int analogPin, float mea_e, float est_e, float q);
    int currentValue = 0;
    int read();
    int getValue();
};
#endif