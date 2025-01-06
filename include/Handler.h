#ifndef HANDLER_H
#define HANDLER_H
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "queueItem.h"

class Handler
{
protected:
    static void startReceiveImpl(void *_this)
    {
        ((Handler *)_this)->receiveTask(_this);
    };
    virtual void receiveTask(void *z) {};
    static void startSendImpl(void *_this)
    {
        ((Handler *)_this)->sendTask(_this);
    };
    virtual void sendTask(void *z) {};

public:
    Handler() {};
    QueueHandle_t receiveQueue = xQueueCreate(10, sizeof(struct QueueItem));
    QueueHandle_t sendQueue = xQueueCreate(10, sizeof(struct QueueItem));
};

#endif
