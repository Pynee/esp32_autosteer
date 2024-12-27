#ifndef GNSSHANDLER_H
#define GNSSHANDLER_H

#include "NMEAParser.h"
#include "configuration.h"
#include "GNSSUART.h"
#include "PandaBuilder.h"

// UART gnssUart;
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