#include "GnssHandler.h"

/* A parser is declared with 3 handlers at most */
NMEAParser<2> parser;
PandaBuilder *pandaBuilderptr;
NMEAMessage nmeaMessage;
UART gnssUart(GNSS_PORT, gnssReceiveData);
bool GGA_Available = false;

void initGnssHandler(PandaBuilder *pandaBuilderptr)
{
  pandaBuilderptr = pandaBuilderptr;
  // the dash means wildcard
  parser.setErrorHandler(errorHandler);
  parser.addHandler("G-GGA", GGA_Handler);
  parser.addHandler("G-VTG", VTG_Handler);

  // xTaskCreate(gnssStreamWorker, "gnssStreamWorker", 3096, NULL, 3, NULL);
}
/* void startWorkerImpl(void *_this)
{
  ((GnssHandler *)_this)->gnssStreamWorker(_this);
} */

/* void gnssStreamWorker(void *z)
{
  for (;;)
  {
    while (Serial2.available())
    {
      parser << Serial2.read();
    }
  }
} */
void gnssReceiveData(uint8_t *data, size_t len)
{
  for (uint8_t i = 0; i < len; i++)
  {
    parser << data[i];
  }
}

void gnssSendData(uint8_t *data, size_t len)
{
  gnssUart.print(*data);
}

// If odd characters showed up.
void errorHandler()
{
  // nothing at the moment
}

void GGA_Handler() // Rec'd GGA
{
  // fix time
  parser.getArg(0, nmeaMessage.fixTime);

  // latitude
  parser.getArg(1, nmeaMessage.latitude);
  parser.getArg(2, nmeaMessage.latNS);

  // longitude
  parser.getArg(3, nmeaMessage.longitude);
  parser.getArg(4, nmeaMessage.lonEW);

  // fix quality
  parser.getArg(5, nmeaMessage.fixQuality);

  // satellite #
  parser.getArg(6, nmeaMessage.numSats);

  // HDOP
  parser.getArg(7, nmeaMessage.HDOP);

  // altitude
  parser.getArg(8, nmeaMessage.altitude);

  // time of last DGPS update
  parser.getArg(12, nmeaMessage.ageDGPS);

  GGA_Available = true;
  pandaBuilderptr->buildPanda(&nmeaMessage);
}

void VTG_Handler()
{
  // vtg heading
  parser.getArg(0, nmeaMessage.vtgHeading);

  // vtg Speed knots
  parser.getArg(4, nmeaMessage.speedKnots);
}