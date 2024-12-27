#ifndef PGNCOMMMANAGER_H
#define PGNCOMMMANAGER_H

#include <ESP.h>
#include <esp_wifi.h>
#include <elapsedMillis.h>
#include "AsyncUDP.h"
#include "configuration.h"
#include "PGNParser.h"
#include "GlobalVariables.h"
#include "UDPPacketManager.h"

class PGNCommManager
{
private:
    static void startsendDataImpl(void *);
    static void startreceiveDataImpl(void *);
    void sendDataTask(void *z);
    void receiveDataTask(void *z);
    // AsyncUDP *udp;
    UDPPacketManager *udpPacketManager;
    PGNParser pgnParser;

public:
    PGNCommManager(UDPPacketManager *udpPacketManager);
    bool init();
    QueueHandle_t managerSendQueue = xQueueCreate(20, sizeof(struct QueueItem));
    QueueHandle_t managerReceiveQueue = xQueueCreate(10, sizeof(struct QueueItem));
};

#endif