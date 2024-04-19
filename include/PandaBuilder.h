#ifndef PANDABUILDER_H
#define PANDABUILDER_H
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "UDPPacketManager.h"
#include "IMUHandler.h"
#include "configuration.h"

// Conversion to Hexidecimal

class PandaBuilder
{
private:
    const char *asciiHex = "0123456789ABCDEF";
    void BuildNmea(void);
    void CalculateChecksum(void);
    // the new PANDA sentence buffer
    char nmea[100];
    UDPPacketManager *packetManager;
    IMUHandler *imuHandler;

public:
    PandaBuilder(UDPPacketManager *packetManager, IMUHandler *imuHandler);
    void buildPanda(NMEAMessage *nmeaMessage);
};
#endif