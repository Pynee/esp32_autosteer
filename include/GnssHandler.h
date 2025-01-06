#ifndef GNSSHANDLER_H
#define GNSSHANDLER_H

#include "NMEAParser.h"
#include "configuration.h"
#include "UART.h"
#include "PandaBuilder.h"
#include "Handler.h"

using namespace std::placeholders;

class GNSSHandler : public Handler
{
private:
    UART gnssUart;
    // extern bool GGA_Available; // Do we have GGA on correct port?
    NMEAParser<2> parser;
    PandaBuilder *pandaBuilderptr;
    NMEAMessage nmeaMessage;
    UART Uart;
    bool GGA_Available = false;

    void receiveTask(void *z) override;

    void sendTask(void *z) override;
    static void errorHandler();
    void GGA_Handler();
    void VTG_Handler();

    // GGA
public:
    GNSSHandler();
    void init(PandaBuilder *pandaBuilder);
    void gnssReceiveData(uint8_t *data, size_t len);
    void gnssSendData(uint8_t *data, size_t len);
};
/*// UART gnssUart;
extern bool GGA_Available; // Do we have GGA on correct port?
void startWorkerImpl(void *);
void errorHandler();
void gnssStreamWorker(void *z);
void GGA_Handler();
void VTG_Handler();

// GGA

void initGnssHandler(PandaBuilder *pandaBuilder);
void gnssReceiveData(uint8_t *data, size_t len);
void gnssSendData(uint8_t *data, size_t len);
*/
#endif