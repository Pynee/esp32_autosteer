
#include "InputHandler.h"

void inputWorker(void *z)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();

  for (;;) // Input Loop
  {
    hardwareSwitches.workSwitch.read();  // read hardware workSwitch
    hardwareSwitches.steerSwitch.read(); // read hardware steerSwitch
    // Put workSwitch in first, steerSwitch in second and remoteSwitch to third bit in switchByte
    hardwareSwitches.switchByte = (hardwareSwitches.remoteSwitch.getState() & 4) & (hardwareSwitches.steerSwitch.getState() & 2) & (hardwareSwitches.workSwitch.getState() & 1);

    sensors.wheelAngleSensor.read(); // update WAS sensor
    if (steerConfig.PressureSensor || steerConfig.CurrentSensor)
    {
      sensors.loadSensor.read(); // update loadSensor/currentSensor
    }

    vTaskDelayUntil(&xLastWakeTime, LOOP_TIME);
  }
}
void initInput()
{
  hardwareSwitches.steerSwitch.init();
  hardwareSwitches.workSwitch.init();

  xTaskCreate(inputWorker, "inputWorker", 3096, NULL, 3, NULL);
}