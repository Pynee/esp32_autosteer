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

  xTaskCreate(this->startWifimanagerWorker, "WifiManagerWorker", 16384, this, 3, NULL);
  xTaskCreate(this->startWorkerImpl, "UdpSendDataTask", 8192, this, 3, NULL);

  return true;
}

void UDPPacketManager::startWorkerImpl(void *_this)
{
  ((UDPPacketManager *)_this)->sendDataTask(_this);
}

// Proxies NTRIP messages to GNSS board serial
void UDPPacketManager::ntripPacketProxy(AsyncUDPPacket packet)
{
  if (packet.length() > 0)
  {
    gnssSendData(packet.data(), packet.length());
  }
}

// Filters packets received from AOGAutoSteerPort (default 8888) before sent to futher processing
void UDPPacketManager::autoSteerPacketParser(AsyncUDPPacket udpPacket)
{
  // Filter most of the wrong packets from futher processing
  if (destinationIPSet && udpPacket.remoteIP() != destinationIP || udpPacket.length() <= 7 || udpPacket.length() > 128)
  {
    Serial.println("Not our packet!");
    return;
  }
  else
  {
    udpPacket.read(this->data, udpPacket.length());

    // Lock receiving messages only from AOG and stop multicasting response messages
    if (!destinationIPSet && this->data[0] == 0x80 && this->data[1] == 0x81 && this->data[2] == 0x7F && this->data[4] == 3 && this->data[5] == 202 && this->data[6] == 202)
    {
      destinationIP = udpPacket.remoteIP();
      destinationIPSet = true;
    }

    QueueItem item = {this->data, udpPacket.length()};
    xQueueSend(pgnCommManager->managerReceiveQueue, &item, (TickType_t)0);
  }
  return;
}

// Task to send data over UDP
void UDPPacketManager::sendDataTask(void *z)
{
  if (sendQueue == NULL)
  {
    Serial.print("queue creation failed!!");
  }
  for (;;)
  {

    QueueItem queueItem;
    std::string *receivedString = nullptr;

    if (xQueueReceive(sendQueue, &queueItem, portMAX_DELAY) == pdTRUE)
    {
      AsyncUDPMessage message = AsyncUDPMessage(queueItem.length);
      message.write(queueItem.data, queueItem.length);
      udp.sendTo(message, destinationIP, DestinationPort);
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
  bool socketsStarted = false;

  WiFiManager wifiManager;
  SetupPage setupPage(&wifiManager);
  setupPage.init();

  wifiManager.setDarkMode(true);
  std::vector<const char *> menu = {"wifi", "info", "param", "sep", "update", "restart", "exit"};
  wifiManager.setMenu(menu); // custom menu, pass vector
  wifiManager.setHostname("ESP32AOG");
  wifiManager.setConfigPortalBlocking(false);
  wifiManager.autoConnect("ESP32AOG");
  wifiManager.setDebugOutput(false);

  // wifiManager.setBreakAfterConfig(true);
  //  wifiManager.setSaveConfigCallback(connectionEstasblished);
  wifiManager.startWebPortal();

  TickType_t xLastWakeTime = xTaskGetTickCount();

  for (;;) // wifi loop
  {
    if (WiFi.status() == WL_CONNECTED && !socketsStarted)
    {
      deviceIP = WiFi.localIP();
      socketsStarted = true;

      // Connected!
      Serial.println("WiFi connected");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());

      if (udp.listen(AOGAutoSteerPort))
      {
        Serial.print("UDP Listening on IP: ");
        Serial.println(WiFi.localIP());
        udp.onPacket([=](AsyncUDPPacket packet)
                     { l_pThis->autoSteerPacketParser(packet); });
      }
      if (ntrip.listen(AOGNtripPort))
      {
        Serial.print("NTRIP passthrough Listening on IP: ");
        Serial.println(WiFi.localIP());
        ntrip.onPacket([=](AsyncUDPPacket packet)
                       { l_pThis->ntripPacketProxy(packet); });
      }
    }
    wifiManager.process();
    xTaskDelayUntil(&xLastWakeTime, WIFIMANAGER_INTERVAL / portTICK_PERIOD_MS);
  }
}
