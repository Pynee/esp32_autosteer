
#include "InputHandler.h"

void inputWorker(void *z)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();

  for (;;) // Input Loop
  {
    // Serial.println("Reading pins...");
    hardwareSwitches.workSwitch.read();  // read hardware workSwitch
    hardwareSwitches.steerSwitch.read(); // read hardware steerSwitch
    // Put workSwitch in first, steerSwitch in second and remoteSwitch to third bit in switchByte
    hardwareSwitches.switchByte = (hardwareSwitches.remoteSwitch.getState() << 2) | (hardwareSwitches.steerSwitch.getState() << 1) | (hardwareSwitches.workSwitch.getState());
    sensors.wheelAngleSensor.read(); // update WAS sensor
    dataToSend.currentSteerAngle = ((double)sensors.wheelAngleSensor.getValue() - 2048.0) / steerSettings.steerSensorCounts - steerSettings.wasOffset;
    if (steerConfig.PressureSensor || steerConfig.CurrentSensor)
    {
      sensors.loadSensor.read(); // update loadSensor/currentSensor
    }

    xTaskDelayUntil(&xLastWakeTime, LOOP_TIME / portTICK_PERIOD_MS);
  }
}
void initInput()
{
  hardwareSwitches.steerSwitch.init();
  hardwareSwitches.workSwitch.init();

  xTaskCreate(inputWorker, "inputWorker", 4096, NULL, 3, NULL);
}