#ifndef PGNPARSER_H
#define PGNPARSER_H

#include <Arduino.h>
#include <ESP.h>
#include <esp_wifi.h>
#include <WiFiManager.h>
#include "AsyncUDP.h"
#include <EEPROM.h>
#include "configuration.h"
// #include "GNSSUART.h"
#include "PGNParser.h"
#include "GlobalVariables.h"
#include "queueItem.h"

class PGNCommManager;

class PGNParser
{
private:
    void steerSettingsInit();
    // uint8_t data[16384];
    // uint8_t *buffer;
    // int bufferIndex = 0;

    // int packetSize;
    PGNCommManager *pgnCommManager;
    IPAddress ipDes = IPAddress(AOGIP); // AOG IP
    bool ipDesIsSet = false;
    static void startParsePacketImpl(void *);
    void packetParserTask(void *z);
    void printLnByteArray(uint8_t *data, uint8_t datalen);
    void parseSteerData(uint8_t *packet);
    void parseSteerSettings(uint8_t *packet);
    void parseSteerConfig(uint8_t *packet);
    uint8_t calculateChecksum(uint8_t *packet, int startPos, int stopPos);

public:
    uint8_t PGN_253[14] = {0x80, 0x81, 126, 0xFD, 8, 0xc7, 0, 0, 0, 0, 0, 0, 0, 0xCC};
    uint8_t helloFromIMU[11] = {0x80, 0x81, 121, 121, 5, 0, 0, 0, 0, 0, 71};
    uint8_t helloFromAutoSteer[11] = {128, 129, 126, 126, 5, 0, 0, 0, 0, 0, 71};
    uint8_t PGN_250[14] = {0x80, 0x81, 126, 0xFA, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0xCC};
    uint8_t scanReply[13] = {128, 129, 126, 203, 7, 0, 0, 0, 0, 0, 0, 0, 23};
    PGNParser(PGNCommManager *commManager);
    QueueHandle_t parseQueue = xQueueCreate(10, sizeof(struct QueueItem));
    void parsePacket(uint8_t *packet, int size);
};
#endif