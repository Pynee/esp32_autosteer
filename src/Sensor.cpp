#include "Sensor.h"

Sensor::Sensor(int analogPin, float mea_e, float est_e, float q) : sensorFilter(mea_e, est_e, q)
{
    pin = analogPin;
}

int Sensor::read()
{
    currentValue = sensorFilter.updateEstimate(analogRead(pin));
    return currentValue;
};
int Sensor::getValue()
{
    return currentValue;
};