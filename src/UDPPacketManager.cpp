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
    if (!destinationIPSet && this->data[0] == 0x80 && this->data[1] == 0x81 && this->data[2] == 0x7F && this->data[3] == 200 && this->data[4] == 3)
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

String UDPPacketManager::handleSetupPage()
{
  String page;
  page += FPSTR(HTTP_HEAD_START);
  page.replace(FPSTR(T_v), "AoG setup");
  page += FPSTR(HTTP_SCRIPT);
  page += FPSTR(HTTP_STYLE);
  page += FPSTR(HTTP_HEAD_END);
  String pitem = "";

  pitem = FPSTR(HTTP_FORM_START);
  pitem.replace(FPSTR(T_v), F("paramsave"));
  page += pitem;
  pitem = FPSTR(HTTP_FORM_SELECT_START);
  pitem.replace(FPSTR(T_v), F("openPage(this)"));
  pitem.replace(FPSTR(T_n), F("board"));
  page = pitem;
  String options[] = {"AIO 5 board", "ESP32 board", "Advanced"};
  for (int i = 0; i < 3; i++)
  {
    pitem = FPSTR(HTTP_FORM_SELECT_OPTION);
    pitem.replace(FPSTR(T_v), String(i));
    pitem.replace(FPSTR(T_t), options[i]);
    if (i == board)
    {
      pitem.replace(FPSTR(T_c), F(" selected"));
    }
    for(int j = 0; j < 3;j++){
      

    }
  }
  for (int i = 0; i < 3; i++)
  {
    page += pitem;
    pitem = FPSTR(HTTP_FORM_TAB);
    pitem.replace(FPSTR(T_n), options[1]);
    if (i != board)
    {
      pitem.replace(FPSTR(T_c), "d-none");
    }

    page = FPSTR(HTTP_DIV_END);
  };

  page = FPSTR(HTTP_FORM_SELECT_END);

  page += FPSTR(HTTP_FORM_END);
  page += FPSTR(HTTP_BACKBTN);
  page += FPSTR(HTTP_END);

  return page;
}

void UDPPacketManager::handleRoute()
{
  Serial.println("[HTTP] handle route Custom");
  wifiManager.server->send(200, FPSTR(HTTP_HEAD_CT), handleSetupPage());
}
void UDPPacketManager::handleNotFound()
{
  Serial.println("[HTTP] override handle route");
  wifiManager.handleNotFound();
}

void UDPPacketManager::bindServerCallback()
{
  wifiManager.server->on("/param", std::bind(&UDPPacketManager::handleRoute, this));

  // you can override wm route endpoints, I have not found a way to remove handlers, but this would let you disable them or add auth etc.
  // wm.server->on("/info",handleNotFound);
  // wm.server->on("/update",handleNotFound);
  wifiManager.server->on("/erase", std::bind(&UDPPacketManager::handleNotFound, this)); // disable erase
}

void UDPPacketManager::wifimanagerWorker(void *z)
{
  UDPPacketManager *pThis = (UDPPacketManager *)z;
  WiFiManager *wifimanager = &pThis->wifiManager;

  // SetupPage setupPage(&wifiManager);
  // setupPage.init();
  wifiManager.setWebServerCallback(std::bind(&UDPPacketManager::bindServerCallback, this));
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
