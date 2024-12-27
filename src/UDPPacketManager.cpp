#include "UDPPacketManager.h"
// Forward declaration
#include "PGNCommManager.h"
#include "SetupPage.h"

UDPPacketManager::UDPPacketManager(void gnssSendData(uint8_t *data, size_t len))
{
  this->gnssSendData = gnssSendData;
}

bool UDPPacketManager::init(PGNCommManager *commManager)
{
  this->pgnCommManager = commManager;
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP

  WiFi.useStaticBuffers(true);
  esp_wifi_set_ps(WIFI_PS_NONE);

  /*if (!wfm.autoConnect("ESP32AGOpenGPS"))
  {
    // Did not connect, print error message
    // Serial.println("failed to connect and hit timeout");
    Serial.println("failed to connect and hit timeout");

    // Reset and try again
    ESP.restart();
    delay(1000);
  }*/

  xTaskCreate(this->startWifimanagerWorker, "WifiManagerWorker", 16384, this, 3, NULL);
  xTaskCreate(this->startWorkerImpl, "UdpSendDataTask", 8192, this, 3, NULL);

  return true;
}

void UDPPacketManager::startWorkerImpl(void *_this)
{
  ((UDPPacketManager *)_this)->sendDataTask(_this);
}

void UDPPacketManager::ntripPacketProxy(AsyncUDPPacket packet)
{
  if (packet.length() > 0)
  {
    // Serial2.write(packet.data(), packet.length());
    gnssSendData(packet.data(), packet.length());
  }
}

void UDPPacketManager::autoSteerPacketParser(AsyncUDPPacket udpPacket)
{
  uint8_t testData[32];
  // Serial.println(udpPacket.length());
  if (destinationIPSet && udpPacket.remoteIP() != destinationIP || udpPacket.length() <= 7 || udpPacket.length() > 128)
  {
    Serial.println("Not our packet!");
    return;
  }
  else
  {

    // Serial.println(udpPacket.length());
    udpPacket.read(this->data, udpPacket.length());
    /*
    Serial.print("Packet: ");
    for (int i = 0; i < udpPacket.length(); i++)
    {
      Serial.print(data[i]);
      Serial.print(", ");
    }

    Serial.println("Copy successful");
    */
    if (!destinationIPSet && this->data[0] == 0x80 && this->data[1] == 0x81 && this->data[2] == 0x7F && this->data[4] == 3 && this->data[5] == 202 && this->data[6] == 202)
    {
      // Set destination ip so we can stop floding the network with broadcasts
      destinationIP = udpPacket.remoteIP();
      destinationIPSet = true;
    }

    QueueItem item = {this->data, udpPacket.length()};
    xQueueSend(pgnCommManager->managerReceiveQueue, &item, (TickType_t)0);
    //(void *)
    // delete &item;
  }
  return;
}

void UDPPacketManager::sendDataTask(void *z)
{
  if (sendQueue == NULL)
  {
    Serial.print("queue creation failed!!");
    // Serial.print("queue creation failed!!");
    //  queue creation failed!!
  }
  for (;;)
  {
    // while (uxQueueMessagesWaiting(sendQueue))
    //{
    QueueItem queueItem;
    std::string *receivedString = nullptr;

    if (xQueueReceive(sendQueue, &queueItem, portMAX_DELAY) == pdTRUE)
    {
      // Serial.println(uxQueueMessagesWaiting(sendQueue));

      // if (queueItem.data[3] == 121 && queueItem.data[2] == 121)
      //{
      //   Serial.println(reinterpret_cast<intptr_t>(queueItem.data + 11));
      //   Serial.println(queueItem.data[10]);
      // }
      AsyncUDPMessage message = AsyncUDPMessage(queueItem.length);
      message.write(queueItem.data, queueItem.length);
      udp.sendTo(message, destinationIP, DestinationPort);
      // delete receivedString;
      //}
    }
  }
  vTaskDelete(NULL);
}

void UDPPacketManager::startWifimanagerWorker(void *_this)
{
  ((UDPPacketManager *)_this)->wifimanagerWorker(_this);
}

void UDPPacketManager::wifimanagerWorker(void *z)
{

  UDPPacketManager *l_pThis = (UDPPacketManager *)z;
  Serial.println((int)&l_pThis);
  bool socketsStarted = false;
  WiFiManager wifiManager;
  std::string testStrings[] = {"1", "2"};
  std::string name = "test";
  std::string testString = inputSelect("test", testStrings, 2, 0);
  WiFiManagerParameter custom_html("<p style=\"font-weight:Bold;\">AgOpenGPS Settings</p>"); // only custom html
  WiFiManagerParameter testSelect(testString.c_str());
  // WiFiManagerParameter custom_mqtt_server("server", "mqtt server", "", 40);
  // WiFiManagerParameter custom_mqtt_port("port", "mqtt port", "", 6);
  // WiFiManagerParameter custom_token("api_token", "api token", "", 16);
  // WiFiManagerParameter custom_tokenb("invalid token", "invalid token", "", 0);                                                  // id is invalid, cannot contain spaces
  // WiFiManagerParameter custom_ipaddress("input_ip", "input IP", "", 15, "pattern='\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}'"); // custom input attrs (ip mask)
  // WiFiManagerParameter custom_input_type("input_pwd", "input pass", "", 15, "type='password'");                                 // custom input attrs (ip mask)
  WiFiManagerParameter custom_send_data("<p style=\"font-weight:Bold;\">Send data to:</p>");
  const char _customHtml_checkbox[] = "type=\"checkbox\"";
  WiFiManagerParameter custom_checkbox_Serial("my_serial", "Serial", "T", 2, _customHtml_checkbox, WFM_LABEL_AFTER);
  WiFiManagerParameter custom_checkbox_Wifi("my_wifi", "WIFI", "T", 2, _customHtml_checkbox, WFM_LABEL_AFTER);
  WiFiManagerParameter custom_checkbox_Ethernet("my_ethernet", "Ethernet", "T", 2, _customHtml_checkbox, WFM_LABEL_AFTER);
  WiFiManagerParameter custom_checkbox_CAN("my_can", "CAN", "T", 2, _customHtml_checkbox, WFM_LABEL_AFTER);
  Serial.print(testSelect.getCustomHTML());
  setupParameters[0].customString = inputSelect(setupParameters[0].name, setupParameters[0].selectStrings, setupParameters[0].selectAmount, setupParameters[0].selected);
  WiFiManagerParameter test(setupParameters[0].customString.c_str());
  wifiManager.addParameter(&test);
  const char *bufferStr = R"(
  <!-- INPUT CHOICE -->
  <br/>
  <p>Select IMU</p>
  <input style='display: inline-block;' type='radio' id='choice1' name='program_selection' value='1'>
  <label for='choice1'>BNO085/BNO055</label><br/>
  <input style='display: inline-block;' type='radio' id='choice2' name='program_selection' value='2'>
  <label for='choice2'>???</label><br/>

  <!-- INPUT SELECT -->
  <br/>
  <label for='input_select'>IMU connection</label>
  <select name="input_select" id="input_select" class="button">
  <option value="0">I2C</option>
  <option value="1" selected>I2C RVC</option>
  <option value="2">SPI</option>
  </select>
  )";

  WiFiManagerParameter custom_html_inputs(bufferStr);
  custom_html_inputs.setValue("", 1);
  // add all your parameters here
  wifiManager.addParameter(&custom_html);
  wifiManager.addParameter(&testSelect);
  // wifiManager.addParameter(&custom_mqtt_server);
  // wifiManager.addParameter(&custom_mqtt_port);
  // wifiManager.addParameter(&custom_token);
  // wifiManager.addParameter(&custom_tokenb);
  // wifiManager.addParameter(&custom_ipaddress);
  wifiManager.addParameter(&custom_send_data);
  wifiManager.addParameter(&custom_checkbox_Serial);
  wifiManager.addParameter(&custom_checkbox_Wifi);
  wifiManager.addParameter(&custom_checkbox_Ethernet);
  wifiManager.addParameter(&custom_checkbox_CAN);
  // wifiManager.addParameter(&custom_input_type);

  wifiManager.addParameter(&custom_html_inputs);

  // set values later if you want
  // custom_html.setValue("test", 4);
  // custom_token.setValue("test", 4);

  // const char* icon = "
  // <link rel='icon' type='image/png' sizes='16x16' href='data:image/png;base64,
  // iVBORw0KGgoAAAANSUhEUgAAABAAAAAQBAMAAADt3eJSAAAAMFBMVEU0OkArMjhobHEoPUPFEBIu
  // O0L+AAC2FBZ2JyuNICOfGx7xAwTjCAlCNTvVDA1aLzQ3COjMAAAAVUlEQVQI12NgwAaCDSA0888G
  // CItjn0szWGBJTVoGSCjWs8TleQCQYV95evdxkFT8Kpe0PLDi5WfKd4LUsN5zS1sKFolt8bwAZrCa
  // GqNYJAgFDEpQAAAzmxafI4vZWwAAAABJRU5ErkJggg==' />";

  // set custom html head content , inside <head>
  // examples of favicon, or meta tags etc
  // const char* headhtml = "<link rel='icon' type='image/png' href='data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADAAAAAwCAYAAABXAvmHAAADQElEQVRoQ+2YjW0VQQyE7Q6gAkgFkAogFUAqgFQAVACpAKiAUAFQAaECQgWECggVGH1PPrRvn3dv9/YkFOksoUhhfzwz9ngvKrc89JbnLxuA/63gpsCmwCADWwkNEji8fVNgotDM7osI/x777x5l9F6JyB8R4eeVql4P0y8yNsjM7KGIPBORp558T04A+CwiH1UVUItiUQmZ2XMReSEiAFgjAPBeVS96D+sCYGaUx4cFbLfmhSpnqnrZuqEJgJnd8cQplVLciAgX//Cf0ToIeOB9wpmloLQAwpnVmAXgdf6pwjpJIz+XNoeZQQZlODV9vhc1Tuf6owrAk/8qIhFbJH7eI3eEzsvydQEICqBEkZwiALfF70HyHPpqScPV5HFjeFu476SkRA0AzOfy4hYwstj2ZkDgaphE7m6XqnoS7Q0BOPs/sw0kDROzjdXcCMFCNwzIy0EcRcOvBACfh4k0wgOmBX4xjfmk4DKTS31hgNWIKBCI8gdzogTgjYjQWFMw+o9LzJoZ63GUmjWm2wGDc7EvDDOj/1IVMIyD9SUAL0WEhpriRlXv5je5S+U1i2N88zdPuoVkeB+ls4SyxCoP3kVm9jsjpEsBLoOBNC5U9SwpGdakFkviuFP1keblATkTENTYcxkzgxTKOI3jyDxqLkQT87pMA++H3XvJBYtsNbBN6vuXq5S737WqHkW1VgMQNXJ0RshMqbbT33sJ5kpHWymzcJjNTeJIymJZtSQd9NHQHS1vodoFoTMkfbJzpRnLzB2vi6BZAJxWaCr+62BC+jzAxVJb3dmmiLzLwZhZNPE5e880Suo2AZgB8e8idxherqUPnT3brBDTlPxO3Z66rVwIwySXugdNd+5ejhqp/+NmgIwGX3Py3QBmlEi54KlwmjkOytQ+iJrLJj23S4GkOeecg8G091no737qvRRdzE+HLALQoMTBbJgBsCj5RSWUlUVJiZ4SOljb05eLFWgoJ5oY6yTyJp62D39jDANoKKcSocPJD5dQYzlFAFZJflUArgTPZKZwLXAnHmerfJquUkKZEgyzqOb5TuDt1P3nwxobqwPocZA11m4A1mBx5IxNgRH21ti7KbAGiyNn3HoF/gJ0w05A8xclpwAAAABJRU5ErkJggg==' />";
  // const char* headhtml = "<meta name='color-scheme' content='dark light'><style></style><script></script>";
  // wm.setCustomHeadElement(headhtml);

  // set custom html menu content , inside menu item "custom", see setMenu()
  // const char *menuhtml = "<form action='/custom' method='get'><button>Settings</button></form><br/>\n";
  // wifiManager.setCustomMenuHTML(menuhtml);

  // invert theme, dark
  wifiManager.setDarkMode(true);

  // show scan RSSI as percentage, instead of signal stength graphic
  // wm.setScanDispPerc(true);

  //  Set cutom menu via menu[] or vector
  //  const char* menu[] = {"wifi","wifinoscan","info","param","close","sep","erase","restart","exit"};
  //  wm.setMenu(menu,9); // custom menu array must provide length

  std::vector<const char *> menu = {"wifi", "info", "param", "sep", "update", "restart", "exit"};
  wifiManager.setMenu(menu); // custom menu, pass vector

  // wifiManager.resetSettings();
  wifiManager.setHostname("ESP32AOG");
  // wifiManager.setEnableConfigPortal(false);
  wifiManager.setConfigPortalBlocking(false);
  wifiManager.autoConnect("ESP32AOG");
  //  Supress Debug information
  wifiManager.setDebugOutput(false);
  // wifiManager.setAPCallback();
  // wifiManager.startConfigPortal();
  TickType_t xLastWakeTime = xTaskGetTickCount();
  wifiManager.setBreakAfterConfig(true);
  // wifiManager.setSaveConfigCallback(connectionEstasblished);
  wifiManager.startWebPortal();

  for (;;) // Input Loop
  {
    if (WiFi.status() == WL_CONNECTED && !socketsStarted)
    {
      // wifiManager.stopWebPortal();
      deviceIP = WiFi.localIP();
      socketsStarted = true;

      // Connected!
      Serial.println("WiFi connected");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      // Serial.println("WiFi connected");
      // Serial.print("IP address: ");
      // Serial.println(WiFi.localIP());

      if (udp.listen(AOGAutoSteerPort))
      {
        Serial.print("UDP Listening on IP: ");
        Serial.println(WiFi.localIP());
        // Serial.print("UDP Listening on IP: ");
        // Serial.println(WiFi.localIP());
        udp.onPacket([=](AsyncUDPPacket packet)
                     { l_pThis->autoSteerPacketParser(packet); });
      }
      if (ntrip.listen(AOGNtripPort))
      {
        Serial.print("NTRIP passthrough Listening on IP: ");
        Serial.println(WiFi.localIP());
        // Serial.print("UDP Listening on IP: ");
        // Serial.println(WiFi.localIP());
        ntrip.onPacket([=](AsyncUDPPacket packet)
                       { l_pThis->ntripPacketProxy(packet); });
      }
    }
    wifiManager.process();
    xTaskDelayUntil(&xLastWakeTime, WIFIMANAGER_INTERVAL / portTICK_PERIOD_MS);
  }
  // wifiManager.stopConfigPortal();
}

std::string UDPPacketManager::inputSelect(std::string name, std::string *strings, int stringArraySize, int selected)
{
  std::string returnStr = "<!-- INPUT SELECT --><br/><label for='input_select'>";
  returnStr.append(name.c_str());
  returnStr.append("</label><select name=\"input_select\" id=\"input_select\" class=\"button\">");

  // String returnString = "<!-- INPUT SELECT --><br/><label for='input_select'>" + name + "IMU connection</label><select name=\"input_select\" id=\"input_select\" class=\"button\">";

  for (int index = 0; index < stringArraySize; index++)
  {
    returnStr.append("<option value=\"");
    returnStr.append(std::to_string(index).c_str());
    returnStr.append("\"");
    if (selected == index)
    {
      returnStr.append(" selected");
    }
    returnStr.append(">");
    returnStr.append(strings[index]);
    returnStr.append("</option>");
  }
  returnStr.append("</select>");
  Serial.println(returnStr.c_str());
  return returnStr;
}