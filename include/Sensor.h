#include <SimpleKalmanFilter.h>
#include <Arduino.h>

class Sensor
{
private:
    int pin = 0;
    int currentValue = 0;
    SimpleKalmanFilter sensorFilter;

public:
    Sensor(int analogPin, float mea_e, float est_e, float q);
    int read();
    int getValue();
};