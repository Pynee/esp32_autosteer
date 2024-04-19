#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>
#include <Arduino.h>
class Button
{

private:
    uint8_t debounce = 0;
    bool buttonState = false;
    bool isChanged = false;
    bool risingEdge = false;

public:
    int pin;
    Button(int inputPin);
    void init();
    bool read();
    bool getState();
    bool stateChanged();
    bool isItRising();
};
#endif