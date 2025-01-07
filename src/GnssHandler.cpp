#include "GnssHandler.h"

GNSSHandler::GNSSHandler() : gnssUart(GNSS_PORT, this)
{
}
/* A parser is declared with 3 handlers at most */
// NMEAParser<2> parser;
// PandaBuilder *pandaBuilderptr;
// NMEAMessage nmeaMessage;
/* A parser is declared with 3 handlers at most */
NMEAParser<2> parser;
PandaBuilder *pandaBuilderptr;
NMEAMessage nmeaMessage;

bool GGA_Available = false;

void GNSSHandler::init(PandaBuilder *pandaBuilder)
{
  gnssUart.init(GNSS_TX_PIN, GNSS_RX_PIN);
  pandaBuilderptr = pandaBuilder;

  parser.setErrorHandler(GNSSHandler::errorHandler);

  parser.addHandler("G-GGA", [this]()
                    { GGA_Handler(); });
  parser.addHandler("G-VTG", [this]()
                    { VTG_Handler(); });
  xTaskCreatePinnedToCore(startReceiveImpl, "gnssreceiveTask", 3096, this, 3, NULL, 1);
  xTaskCreatePinnedToCore(startSendImpl, "gnssSendTask", 3096, this, 3, NULL, 1);
}
//[this]{VTG_Handler();};
// std::bind(GNSSHandler::GGA_Handler,this)
void GNSSHandler::receiveTask(void *z)
{
  if (receiveQueue == NULL)
  {
    Serial.print("queue creation failed!!");
  }
  for (;;)
  {
    QueueItem queueItem;
    if (xQueueReceive(receiveQueue, &queueItem, portMAX_DELAY) == pdTRUE)
    {
      for (uint8_t i = 0; i < queueItem.length; i++)
      {
        parser << queueItem.data[i];
      }
    }
  }
}
void GNSSHandler::sendTask(void *z)
{
  if (sendQueue == NULL)
  {
    Serial.print("queue creation failed!!");
  }
  for (;;)
  {
    QueueItem queueItem;
    if (xQueueReceive(sendQueue, &queueItem, portMAX_DELAY) == pdTRUE)
    {
      gnssUart.write(queueItem.data, queueItem.length);
    }
  }
}

// If odd characters showed up.
void GNSSHandler::errorHandler()
{
  // nothing at the moment
}

void GNSSHandler::GGA_Handler() // Rec'd GGA
{

  //  fix time
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

  // GGA_Available = true;
  pandaBuilderptr->buildPanda(&nmeaMessage);
}

void GNSSHandler::VTG_Handler()
{
  // vtg heading
  parser.getArg(0, nmeaMessage.vtgHeading);

  // vtg Speed knots
  parser.getArg(4, nmeaMessage.speedKnots);
}