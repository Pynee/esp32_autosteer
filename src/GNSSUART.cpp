#include "GNSSUART.h"

GNSSUART::GNSSUART()
{
}

GNSSUART::GNSSUART(uint8_t _port)
{
    this->port = _port;
}
GNSSUART::GNSSUART(uint8_t _port, void _callback(uint8_t *data, size_t len))
{
    this->port = _port;
    this->callback = _callback;
}

void GNSSUART::init()
{
    init(UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

void GNSSUART::init(int TX_PIN, int RX_PIN)
{
    // esp_log_level_set(TAG, ESP_LOG_INFO);

    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};
    // Install UART driver, and get the queue.
    uart_driver_install(port, UART_BUFFER_SIZE * 2, UART_BUFFER_SIZE * 2, 20, &uartQueue, 0);
    uart_param_config(port, &uart_config);

    // Set UART log level
    // esp_log_level_set(TAG, ESP_LOG_INFO);
    // Set UART pins (using UART0 default pins ie no changes.)
    uart_set_pin(port, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    /* Set pattern interrupt, used to detect the end of a line */
    uart_enable_pattern_det_baud_intr(port, '\n', 1, 9, 0, 0);
    /* Set pattern queue size */
    uart_pattern_queue_reset(port, 10);
    uart_flush(port);

    // Create a task to handler UART event from ISR
    xTaskCreate(startWorkerImpl, "uart_event_task", 4096, this, 12, NULL);
}

void GNSSUART::setCallback(void callback(uint8_t *data, size_t len))
{
    this->callback = callback;
};

void GNSSUART::startWorkerImpl(void *_this)
{
    ((GNSSUART *)_this)->uartEventWorker(_this);
}

void GNSSUART::uartEventWorker(void *pvParameters)
{
    uart_event_t event;
    size_t buffered_size;
    uint8_t dataBuffer[UART_BUFFER_SIZE];
    for (;;)
    {
        // Waiting for UART event.
        if (xQueueReceive(uartQueue, (void *)&event, (TickType_t)portMAX_DELAY))
        {
            bzero(dataBuffer, UART_BUFFER_SIZE);
            // ESP_LOGI(TAG, "uart[%d] event:", uart_port);
            switch (event.type)
            {
            // Event of UART receving data
            /*We'd better handler data event fast, there would be much more data events than
            other types of events. If we take too much time on data event, the queue might
            be full.*/
            case UART_DATA:
            {
                break;
            }
            // Event of HW FIFO overflow detected
            case UART_FIFO_OVF:
            {
                // ESP_LOGI(TAG, "hw fifo overflow");
                //  If fifo overflow happened, you should consider adding flow control for your application.
                //  The ISR has already reset the rx FIFO,
                //  As an example, we directly flush the rx buffer here in order to read more data.
                uart_flush_input(port);
                xQueueReset(uartQueue);
                break;
            }
            // Event of UART ring buffer full
            case UART_BUFFER_FULL:
            {
                // ESP_LOGI(TAG, "ring buffer full");
                //  If buffer full happened, you should consider increasing your buffer size
                //  As an example, we directly flush the rx buffer here in order to read more data.
                uart_flush_input(port);
                xQueueReset(uartQueue);
                break;
            }
            // Event of UART RX break detected
            case UART_BREAK:
            {
                // ESP_LOGI(TAG, "uart rx break");
                break;
            }
            // Event of UART parity check error
            case UART_PARITY_ERR:
            {
                // ESP_LOGI(TAG, "uart parity error");
                break;
            }
            // Event of UART frame error
            case UART_FRAME_ERR:
            {
                // ESP_LOGI(TAG, "uart frame error");
                break;
            }
            // UART_PATTERN_DET
            case UART_PATTERN_DET:
            {
                int pos = uart_pattern_pop_pos(port);
                if (pos != -1)
                {
                    /* read one line(include '\n') */
                    int dataLength = uart_read_bytes(port, dataBuffer, pos + 1, 100 / portTICK_PERIOD_MS);
                    /* make sure the line is a standard string */
                    dataBuffer[dataLength] = '\0';
                    /* Send new line to handle */
                    // Serial.write(dataBuffer, dataLength);
                    // Serial.println();
                    callback(dataBuffer, dataLength);
                }
                else
                {
                    Serial.print("Pattern Queue Size too small");
                    uart_flush_input(port);
                }
                break;
            }
            // Others
            default:
            {
                Serial.printf("uart event type: %d", event.type);
                break;
            }
            }
        }
    }
    free(dataBuffer);
    // dataBuffer = NULL;
    vTaskDelete(NULL);
}

size_t GNSSUART::write(uint8_t c)
{
    uart_write_bytes(port, (const char *)c, sizeof(c));
    return 1;
}
