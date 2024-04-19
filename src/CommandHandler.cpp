
#include "CommandHandler.h"
String cmd = "";

void commandHandler(uint8_t *data, size_t len)
{
  if (Serial.available() > 0)
  {
    // read the incoming string:
    String cmd = String((char *)data, len);

    // prints the received data
    Serial.print("I received: ");
    Serial.println(cmd);

    if (cmd.startsWith("wifireset"))
    {
      Serial.println("Wifi settings reseted. ESP restarting..................");
      // wfm.resetSettings();
      ESP.restart();
    }
    else if (cmd.startsWith("help"))
    {
      Serial.println("wifireset  : Reset wifi settings.");
    }
    else
    {
      Serial.println("Unknown command!");
    }
  }
}