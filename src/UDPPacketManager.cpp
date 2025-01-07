#include "UDPPacketManager.h"
// Forward declaration
#include "PGNCommManager.h"
#include "SetupPage.h"
#include "GnssHandler.h"

UDPPacketManager::UDPPacketManager(GNSSHandler *gnssHandler)
{
  this->gnssHandler = gnssHandler;
}

bool UDPPacketManager::init(PGNCommManager *commManager)
{
  this->pgnCommManager = commManager;
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP

  WiFi.useStaticBuffers(true);
  esp_wifi_set_ps(WIFI_PS_NONE);

  // esp_event_handler_instance_t instance_any_id;
  esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &UDPPacketManager::eventHandler, this, NULL);
  esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &UDPPacketManager::eventHandler, this, NULL);

  xTaskCreatePinnedToCore(this->startWifimanagerWorker, "WifiManagerWorker", 8192, this, 3, NULL, 1);
  xTaskCreatePinnedToCore(this->startWorkerImpl, "UdpSendDataTask", 8192, this, 3, NULL, 1);

  return true;
}

void UDPPacketManager::startWorkerImpl(void *_this)
{
  ((UDPPacketManager *)_this)->sendDataTask(_this);
}

// Proxies NTRIP messages to GNSS board serial port
void UDPPacketManager::ntripPacketProxy(AsyncUDPPacket packet)
{
  if (packet.length() > 0)
  {
    packet.read(this->ntripBuffer, packet.length());
    // gnssSendData(this->ntripBuffer, packet.length());
    QueueItem item = {this->ntripBuffer, packet.length()};
    // Serial.print("received: ");
    // Serial.println(packet.length());
    xQueueSend(gnssHandler->sendQueue, &item, (TickType_t)0);
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
    udpPacket.read(this->data, min((size_t)128, udpPacket.length()));

    // Lock receiving messages only from AOG and stop broadcasting response messages
    if (!destinationIPSet && this->data[0] == 0x80 && this->data[1] == 0x81 && this->data[2] == 0x7F && this->data[4] == 3 && this->data[5] == 202 && this->data[6] == 202)
    {
      Serial.println("Destinationset!");
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

    if (xQueueReceive(sendQueue, &queueItem, portMAX_DELAY) == pdTRUE)
    {
      AsyncUDPMessage message = AsyncUDPMessage(queueItem.length);
      message.write(queueItem.data, queueItem.length);
      udp.sendTo(message, destinationIP, DestinationPort);
    }
  }
  vTaskDelete(NULL);
}

void UDPPacketManager::eventHandler(void *arguments, esp_event_base_t eventBase,
                                    int32_t eventID, void *eventData)
{
  UDPPacketManager *l_pThis = (UDPPacketManager *)arguments;
  if (eventBase == WIFI_EVENT && eventID == WIFI_EVENT_STA_START)
  {
    // esp_wifi_connect();
  }
  else if (eventBase == WIFI_EVENT && eventID == WIFI_EVENT_STA_DISCONNECTED)
  {
    // esp_wifi_connect();
    destinationIPSet = false;
    // s_retry_num++;
    // ESP_LOGI(TAG, "retry to connect to the AP");
    // xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
  }
  else if (eventBase == IP_EVENT && eventID == IP_EVENT_STA_GOT_IP)
  {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)eventData;
    // ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
    // s_retry_num = 0;
    // xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    deviceIP = event->ip_info.ip.addr;
    Serial.println(deviceIP);
    // Connected!
    if (l_pThis->ntrip.listen(AOGNtripPort))
    {
      Serial.print("NTRIP passthrough Listening on IP: ");
      Serial.println(WiFi.localIP());
      l_pThis->ntrip.onPacket([=](AsyncUDPPacket packet)
                              { l_pThis->ntripPacketProxy(packet); });
    }
    if (l_pThis->udp.listen(AOGAutoSteerPort))
    {
      Serial.print("UDP Listening on IP: ");
      Serial.println(WiFi.localIP());
      l_pThis->udp.onPacket([=](AsyncUDPPacket packet)
                            { l_pThis->autoSteerPacketParser(packet); });
    }
  }
}

void UDPPacketManager::startWifimanagerWorker(void *_this)
{
  ((UDPPacketManager *)_this)->wifimanagerWorker(_this);
}

void UDPPacketManager::wifimanagerWorker(void *z)
{
  bool socketsStarted = false;

  WiFiManager wifiManager;
  SetupPage setupPage(&wifiManager);
  setupPage.init();

  wifiManager.setDarkMode(true);
  std::vector<const char *> menu = {"wifi", "info", "param", "sep", "update", "restart", "exit"};
  wifiManager.setMenu(menu); // custom menu, pass vector
  wifiManager.setHostname("esp32aog");
  wifiManager.setConfigPortalBlocking(false);
  wifiManager.autoConnect("ESP32AOG");
  wifiManager.setDebugOutput(false);

  // wifiManager.setBreakAfterConfig(true);
  //  wifiManager.setSaveConfigCallback(connectionEstasblished);
  wifiManager.startWebPortal();

  TickType_t xLastWakeTime = xTaskGetTickCount();

  for (;;) // wifi loop
  {
    wifiManager.process();
    xTaskDelayUntil(&xLastWakeTime, WIFIMANAGER_INTERVAL / portTICK_PERIOD_MS);
  }
}
