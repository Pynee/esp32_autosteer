#ifndef UDPPACKETMANAGER_H
#define UDPPACKETMANAGER_H

#include <Arduino.h>
#include <ESP.h>
#include <freertos/FreeRTOS.h>
#include <esp_wifi.h>
#include <elapsedMillis.h>
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
    void (*gnssSendData)(uint8_t *data, size_t len);
    std::string inputSelect(std::string name, std::string *strings, int stringArraySize, int selected);

public:
    UDPPacketManager(void gnssSendData(uint8_t *data, size_t len));
    bool init(PGNCommManager *commManager);
    // std::string dataString;
    uint8_t data[128] = {49};
    QueueHandle_t sendQueue = xQueueCreate(20, sizeof(struct QueueItem));
    // QueueItem queueItem;
};

#endif