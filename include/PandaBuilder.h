#ifndef PANDABUILDER_H
#define PANDABUILDER_H
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "UDPPacketManager.h"
#include "IMUHandler.h"
#include "configuration.h"
#include "PGNCommManager.h"

class PandaBuilder
{
private:
    const char *asciiHex = "0123456789ABCDEF";
    // void BuildNmea(void);
    uint8_t calculateChecksum(char *packet);

    PGNCommManager *pgnmanager;
    // IMUHandler *imuHandler;
    unsigned long prevTime;

public:
    // the new PANDA sentence buffer
    char nmea[100];
    PandaBuilder(PGNCommManager *PGNManager);
    void buildPanda(NMEAMessage *nmeaMessage);
};
#endif