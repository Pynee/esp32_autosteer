#ifndef UDPPACKETMANAGER_H
#define UDPPACKETMANAGER_H

#include <Arduino.h>
#include <ESP.h>
#include <esp_wifi.h>
#include <elapsedMillis.h>
#include <WiFiManager.h>
#include "AsyncUDP.h"
#include <EEPROM.h>
#include "configuration.h"
#include "UARTHandler.h"

class UDPPacketManager
{
private:
    // UART gnssUART;
    IPAddress myip;
    uint8_t aog2Count = 0;
    WiFiManager wfm;
    elapsedMillis gpsSpeedUpdateTimer = 0;
    // Relays
    bool isRelayActiveHigh = true;
    uint8_t relay = 0, relayHi = 0, uTurn = 0;
    uint8_t tram = 0;

    uint8_t data[16384];
    uint8_t *buffer;
    int bufferIndex = 0;

    // Heart beat hello AgIO
    uint8_t helloFromIMU[11] = {128, 129, 121, 121, 5, 0, 0, 0, 0, 0, 71};
    uint8_t helloFromAutoSteer[11] = {0x80, 0x81, 126, 126, 5, 0, 0, 0, 0, 0, 71};

    // fromAutoSteerData FD 253 - ActualSteerAngle*100 -5,6, SwitchByte-7, pwmDisplay-8
    uint8_t PGN_253[14] = {0x80, 0x81, 126, 0xFD, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0xCC};

    // fromAutoSteerData FA 250 - sensor values etc
    uint8_t PGN_250[14] = {0x80, 0x81, 126, 0xFA, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0xCC};

    int packetSize;

    IPAddress ipDes = IPAddress(AOGIP); // AOG IP
    bool ipDesIsSet = false;

    AsyncUDP udp;
    AsyncUDP ntrip;

    void steerSettingsInit();
    void ntripPacketProxy(AsyncUDPPacket packet);
    void autoSteerPacketParser(AsyncUDPPacket udpPacket);
    void parsePacket(uint8_t *packet, int size, AsyncUDPPacket udpPacket);
    void printLnByteArray(uint8_t *data, uint8_t datalen);
    void parseSteerData(uint8_t *packet);
    void parseSteerSettings(uint8_t *packet);
    void parseSteerConfig(uint8_t *packet);
    static void startWorkerImpl(void *);
    void sendDataTask(void *z);
    void (*gnssSendData)(uint8_t *data, size_t len);

public:
    UDPPacketManager(void gnssSendData(uint8_t *data, size_t len));
    bool initUDP();
    QueueHandle_t sendQueue = xQueueCreate(10, sizeof(struct QueueItem *));
};

#endif