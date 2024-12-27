
#include "CommandHandler.h"
String cmd = "";

void commandHandler(uint8_t *data, size_t len)
{
  if (true) // Serial.available() > 0)
  {
    // read the incoming string:
    String cmd = String((char *)data, len);

    // prints the received data
    Serial.print("I received: ");
    Serial.println(cmd);

    if (cmd.startsWith("wifireset"))
    {
      // Serial.println("Wifi settings reseted. ESP restarting..................");
      //  wfm.resetSettings();
      ESP.restart();
    }
    else if (cmd.startsWith("help"))
    {
      Serial.println("wifireset  : Reset wifi settings.");
    }
    else if (cmd.startsWith("input"))
    {
      Serial.print("remoteSwitch: ");
      Serial.println(hardwareSwitches.remoteSwitch.getState());
      Serial.print("workSwitch: ");
      Serial.println(hardwareSwitches.workSwitch.getState());
      Serial.print("steerSwitch: ");
      Serial.println(hardwareSwitches.steerSwitch.getState());
      Serial.print("WAS Sensor:");
      Serial.println(sensors.wheelAngleSensor.getValue());
    }
    else if (cmd.startsWith("imu"))
    {
      Serial.print("IMU: heading[");
      Serial.print(dataToSend.heading);
      Serial.print("] roll[");
      Serial.print((dataToSend.roll));
      Serial.print("] Pitch[");
      Serial.print(dataToSend.pitch);
      Serial.print("] YawRate[");
      Serial.print(dataToSend.yawRate);
      Serial.println("]");
    }
    else
    {
      Serial.println("Unknown command!");
    }
  }
}