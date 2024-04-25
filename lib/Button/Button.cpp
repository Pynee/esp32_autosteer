#include "Button.h"

Button::Button(int inputPin)
{
    pin = inputPin;
}

// Init button
void Button::init()
{
    pinMode(pin, INPUT_PULLUP);
}

// function to clean out any debounce in physical buttons, first variable is fo
bool Button::read()
{
    // stateBytes variables First bit stores the current stateBytes and the rest of the bits are used for the debounce 1b00000000
    //                                                                              button's current stateBytes   --^ ^--the rest of the bits are for the debounce
    //                                                                   keeps track if the stateBytes have changed--^

    debounce = ((debounce << 1) | (digitalRead(pin) & 1));
    // Serial.println(digitalRead(pin));
    if ((debounce & 15) == 7)
    {
        if (!buttonState)
        {
            Serial.println(buttonState);
            buttonState = true;
            Serial.println(getState());
            isChanged = true;
            risingEdge = true;
        }
        Serial.println("true");
        // stateBytes = stateBytes | 192;
    }
    else if ((debounce & 15) == 8)
    {
        if (buttonState)
        {
            Serial.println("falsee");
            buttonState = false;
            isChanged = true;
            risingEdge = false;
        }
        Serial.println("false");
    }
    return buttonState;
}
// Return button's current state
bool Button::getState()
{
    return !buttonState;
}
// Returns true if state has Changed since last check and reset the changed to false
bool Button::stateChanged()
{
    if (isChanged)
    {
        isChanged = false;
        return true;
    }
    return false;
}
// return true if the last edge detected was rising
bool Button::isItRising()
{
    return !risingEdge;
}