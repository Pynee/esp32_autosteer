#ifndef QUEUEITEM_H
#define QUEUEITEM_H
#include <cstdint>

struct QueueItem
{
    uint8_t *data;
    unsigned int length;
};
#endif