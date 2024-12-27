#include "PGNCommManager.h"

PGNCommManager::PGNCommManager(UDPPacketManager *udpPacketManager) : pgnParser(this)
{
    this->udpPacketManager = udpPacketManager;
}
bool PGNCommManager::init()
{

    xTaskCreate(this->startsendDataImpl, "PGNCMSendTask", 4096, this, 3, NULL);
    xTaskCreate(this->startreceiveDataImpl, "PGNCMRevTask", 4096, this, 3, NULL);
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
        // Serial.print("queue creation failed!!");
        //  queue creation failed!!
    }
    for (;;)
    {
        // while (uxQueueMessagesWaiting(managerSendQueue))
        //{
        QueueItem queueItem;
        // std::string *receivedString = nullptr;

        if (xQueueReceive(managerSendQueue, &queueItem, portMAX_DELAY) == pdTRUE)
        {
            // Serial.println("Sending!");
            if (useSerial)
            {
            }

            if (useWifi | useEthernet & udpPacketManager != nullptr)
            {
                // Serial.println(reinterpret_cast<intptr_t>(queueItem.data + 11));
                // Serial.println(queueItem.data[10]);
                //  Serial.println("Sendin to wifi/ethernet");
                //  Serial.write(queueItem.data, queueItem.length);
                xQueueSend(udpPacketManager->sendQueue, &queueItem, (TickType_t)0);
            }

            if (useCan)
            {
            }
            // delete receivedString;
        }
        //}
    }
}

void PGNCommManager::receiveDataTask(void *z)
{
    if (managerReceiveQueue == NULL)
    {
        Serial.print("queue creation failed!!");
        // Serial.print("queue creation failed!!");
        //  queue creation failed!!
    }
    // while (uxQueueMessagesWaiting(managerReceiveQueue))
    //{

    // std::string *receivedString = nullptr;
    for (;;)
    {
        QueueItem queueItem;
        if (xQueueReceive(managerReceiveQueue, &queueItem, portMAX_DELAY) == pdTRUE)
        {
            // Serial.println("received!!!");
            //  Serial.println(receivedString != nullptr);
            //  Serial.println(*queueItem.data);
            // Serial.print(" Packet length: ");
            // Serial.println(queueItem.length);
            // Serial.write(queueItem.data, queueItem.length);
            // xQueueSend(pgnParser.parseQueue, &queueItem, (TickType_t)0);
            // delete receivedString;
            // delete &queueItem;
            pgnParser.parsePacket(queueItem.data, queueItem.length);
        }
    }
}
