#ifndef UDPPACKETMANAGER_H
#define UDPPACKETMANAGER_H

#include <Arduino.h>
#include <ESP.h>
#include <freertos/FreeRTOS.h>
#include <esp_wifi.h>
#include <ESPmDNS.h>
#include <WiFiManager.h>
#include "AsyncUDP.h"
#include <EEPROM.h>
#include "configuration.h"
// #include "GNSSUART.h"
#include "PGNParser.h"
#include "GlobalVariables.h"
#include "queueItem.h"

class PGNCommManager;
class GNSSHandler;

class UDPPacketManager
{
private:
    // UART gnssUART;
    // uint8_t data[16384];

    uint8_t *buffer;
    int bufferIndex = 0;

    int packetSize;

    IPAddress ipDes = IPAddress(AOGIP); // AOG IP
    bool ipDesIsSet = false;

    PGNCommManager *pgnCommManager;
    // AsyncUDP udp;
    AsyncUDP udp;
    AsyncUDP ntrip;

    void ntripPacketProxy(AsyncUDPPacket packet);
    void autoSteerPacketParser(AsyncUDPPacket udpPacket);
    static void startWorkerImpl(void *);
    void sendDataTask(void *z);
    static void startWifimanagerWorker(void *);
    void wifimanagerWorker(void *z);
    GNSSHandler *gnssHandler;
    std::string inputSelect(std::string name, std::string *strings, int stringArraySize, int selected);

public:
    UDPPacketManager(GNSSHandler *gnssHandler);
    static void eventHandler(void *arguments, esp_event_base_t eventBase,
                             int32_t eventID, void *eventData);
    bool init(PGNCommManager *commManager);
    uint8_t ntripBuffer[512];
    uint8_t data[128] = {49};
    // Queue to send data over UDP
    QueueHandle_t sendQueue = xQueueCreate(20, sizeof(struct QueueItem));
};

#endif