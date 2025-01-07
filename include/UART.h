#ifndef UARTHANDLER_H
#define UARTHANDLER_H

/* UART Events Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
// #include "esp_log.h"
#include "Print.h"
#include "queueItem.h"
#include "Handler.h"

static const char *TAG = "uart_events";

/**
 * This example shows how to use the UART driver to handle special UART events.
 *
 * It also reads data from UART0 directly, and echoes it to console.
 *
 * - Port: UART0
 * - Receive (Rx) buffer: on
 * - Transmit (Tx) buffer: off
 * - Flow control: off
 * - Event queue: on
 * - Pin assignment: TxD (default), RxD (default)
 */

#define EX_UART_NUM UART_NUM_0
#define PATTERN_CHR_NUM (3) /*!< Set the number of consecutive and identical characters received by receiver which defines a UART pattern*/

#define UART_BUFFER_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)

class UART : public Print
{
private:
    uint8_t port = 0;
    Handler *handler;
    static void startWorkerImpl(void *);
    void uartEventWorker(void *pvParameters);
    QueueHandle_t uartQueue;
    void (*callback)(uint8_t *data, size_t len);

public:
    UART();
    UART(uint8_t port);
    UART(uint8_t port, Handler *handler);
    void init();
    void init(int TX_PIN, int RX_PIN);
    void setCallback(void callback(uint8_t *data, size_t len));
    virtual size_t write(uint8_t) override;
    virtual size_t write(const uint8_t *buffer, size_t size) override;
    inline size_t write(unsigned long n) { return write((uint8_t)n); }
    inline size_t write(long n) { return write((uint8_t)n); }
    inline size_t write(unsigned int n) { return write((uint8_t)n); }
    inline size_t write(int n) { return write((uint8_t)n); }
    using Print::write;
    // pull in write(str) and write(buf, size) from Print
};
#endif