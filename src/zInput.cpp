#include <SimpleKalmanFilter.h>
#include <Arduino.h>
#include "configuration.h"
#include "Button.h"

void initInput()
{
  hardwareSwitches.steerSwitch.init();
  hardwareSwitches.workSwitch.init();
}

void inputHandler()
{
  hardwareSwitches.workSwitch.read();  // read hardware workSwitch
  hardwareSwitches.steerSwitch.read(); // read hardware steerSwitch
  // Put workSwitch in first, steerSwitch in second and remoteSwitch to third bit in switchByte
  hardwareSwitches.switchByte = (hardwareSwitches.remoteSwitch.getState() & 4) & (hardwareSwitches.steerSwitch.getState() & 2) & (hardwareSwitches.workSwitch.getState() & 1);

  sensors.wheelAngleSensor.read(); // update WAS sensore
  if (steerConfig.PressureSensor || steerConfig.CurrentSensor)
  {
    sensors.loadSensor.read(); // update loadSensor/currentSensor
    if (steerConfig.PressureSensor)
    {
    }
  }
}
