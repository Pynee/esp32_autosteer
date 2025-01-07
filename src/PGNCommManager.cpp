#include "PGNCommManager.h"

PGNCommManager::PGNCommManager(UDPPacketManager *udpPacketManager) : pgnParser(this)
{
    this->udpPacketManager = udpPacketManager;
}
bool PGNCommManager::init()
{

    xTaskCreatePinnedToCore(this->startsendDataImpl, "PGNCMSendTask", 4096, this, 3, NULL, 1);
    xTaskCreatePinnedToCore(this->startreceiveDataImpl, "PGNCMRevTask", 4096, this, 3, NULL, 1);
    return true;
}
void PGNCommManager::startsendDataImpl(void *_this)
{
    ((PGNCommManager *)_this)->sendDataTask(_this);
}
void PGNCommManager::startreceiveDataImpl(void *_this)
{
    ((PGNCommManager *)_this)->receiveDataTask(_this);
}

void PGNCommManager::sendDataTask(void *z)
{
    if (managerSendQueue == NULL)
    {
        Serial.print("queue creation failed!!");
    }
    for (;;)
    {
        QueueItem queueItem;
        if (xQueueReceive(managerSendQueue, &queueItem, portMAX_DELAY) == pdTRUE)
        {
            if (useSerial)
            {
            }

            if (useWifi | useEthernet & udpPacketManager != nullptr)
            {
                xQueueSend(udpPacketManager->sendQueue, &queueItem, (TickType_t)0);
            }

            if (useCan)
            {
            }
        }
    }
}

void PGNCommManager::receiveDataTask(void *z)
{
    if (managerReceiveQueue == NULL)
    {
        Serial.print("queue creation failed!!");
    }
    for (;;)
    {
        QueueItem queueItem;
        if (xQueueReceive(managerReceiveQueue, &queueItem, portMAX_DELAY) == pdTRUE)
        {
            pgnParser.parsePacket(queueItem.data, queueItem.length);
        }
    }
}
