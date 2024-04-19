#ifndef GNSSHANDLER_H
#define GNSSHANDLER_H

#include "zNMEAParser.h"
#include "configuration.h"
#include "UARTHandler.h"
#include "PandaBuilder.h"

/* A parser is declared with 3 handlers at most */
extern NMEAParser<2> parser;
extern PandaBuilder *pandaBuilderptr;
extern NMEAMessage nmeaMessage;
extern UART gnssUart;
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

#endif