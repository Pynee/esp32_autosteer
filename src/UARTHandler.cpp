#include "UARTHandler.h"

UART::UART()
{
}

UART::UART(uint8_t port)
{
    this->uart_port = port;
}
UART::UART(uint8_t port, void callback(uint8_t *data, size_t len))
{
    this->uart_port = port;
    this->callback = callback;
}
void UART::setCallback(void callback(uint8_t *data, size_t len))
{
    this->callback = callback;
};

void UART::startWorkerImpl(void *_this)
{
    ((UART *)_this)->uartEventWorker(_this);
}

void UART::uartEventWorker(void *pvParameters)
{
    uart_event_t event;
    size_t buffered_size;
    uint8_t *dtmp = (uint8_t *)malloc(RD_BUF_SIZE);
    for (;;)
    {
        // Waiting for UART event.
        if (xQueueReceive(uart_queue, (void *)&event, (TickType_t)portMAX_DELAY))
        {
            bzero(dtmp, RD_BUF_SIZE);
            ESP_LOGI(TAG, "uart[%d] event:", uart_port);
            switch (event.type)
            {
            // Event of UART receving data
            /*We'd better handler data event fast, there would be much more data events than
            other types of events. If we take too much time on data event, the queue might
            be full.*/
            case UART_DATA:
            {
                ESP_LOGI(TAG, "[UART DATA]: %d", event.size);
                uart_read_bytes(uart_port, dtmp, event.size, portMAX_DELAY);
                callback(dtmp, event.size);
                ESP_LOGI(TAG, "[DATA EVT]:");
                uart_write_bytes(uart_port, (const char *)dtmp, event.size);
                break;
            }
            // Event of HW FIFO overflow detected
            case UART_FIFO_OVF:
            {
                ESP_LOGI(TAG, "hw fifo overflow");
                // If fifo overflow happened, you should consider adding flow control for your application.
                // The ISR has already reset the rx FIFO,
                // As an example, we directly flush the rx buffer here in order to read more data.
                uart_flush_input(uart_port);
                xQueueReset(uart_queue);
                break;
            }
            // Event of UART ring buffer full
            case UART_BUFFER_FULL:
            {
                ESP_LOGI(TAG, "ring buffer full");
                // If buffer full happened, you should consider increasing your buffer size
                // As an example, we directly flush the rx buffer here in order to read more data.
                uart_flush_input(uart_port);
                xQueueReset(uart_queue);
                break;
            }
            // Event of UART RX break detected
            case UART_BREAK:
            {
                ESP_LOGI(TAG, "uart rx break");
                break;
            }
            // Event of UART parity check error
            case UART_PARITY_ERR:
            {
                ESP_LOGI(TAG, "uart parity error");
                break;
            }
            // Event of UART frame error
            case UART_FRAME_ERR:
            {
                ESP_LOGI(TAG, "uart frame error");
                break;
            }
            // UART_PATTERN_DET
            case UART_PATTERN_DET:
            {
                uart_get_buffered_data_len(uart_port, &buffered_size);
                int pos = uart_pattern_pop_pos(uart_port);
                ESP_LOGI(TAG, "[UART PATTERN DETECTED] pos: %d, buffered size: %d", pos, buffered_size);
                if (pos == -1)
                {
                    // There used to be a UART_PATTERN_DET event, but the pattern position queue is full so that it can not
                    // record the position. We should set a larger queue size.
                    // As an example, we directly flush the rx buffer here.
                    uart_flush_input(uart_port);
                }
                else
                {
                    uart_read_bytes(uart_port, dtmp, pos, 100 / portTICK_PERIOD_MS);
                    uint8_t pat[PATTERN_CHR_NUM + 1];
                    memset(pat, 0, sizeof(pat));
                    uart_read_bytes(uart_port, pat, PATTERN_CHR_NUM, 100 / portTICK_PERIOD_MS);
                    ESP_LOGI(TAG, "read data: %s", dtmp);
                    ESP_LOGI(TAG, "read pat : %s", pat);
                }
                break;
            }
            // Others
            default:
            {
                ESP_LOGI(TAG, "uart event type: %d", event.type);
                break;
            }
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}

void UART::init(void)
{
    esp_log_level_set(TAG, ESP_LOG_INFO);

    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};
    // Install UART driver, and get the queue.
    uart_driver_install(uart_port, BUF_SIZE * 2, BUF_SIZE * 2, 20, &uart_queue, 0);
    uart_param_config(uart_port, &uart_config);

    // Set UART log level
    esp_log_level_set(TAG, ESP_LOG_INFO);
    // Set UART pins (using UART0 default pins ie no changes.)
    uart_set_pin(uart_port, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    // Set uart pattern detect function.
    uart_enable_pattern_det_baud_intr(uart_port, '+', PATTERN_CHR_NUM, 9, 0, 0);
    // Reset the pattern queue length to record at most 20 pattern positions.
    uart_pattern_queue_reset(uart_port, 20);

    // Create a task to handler UART event from ISR
    xTaskCreate(startWorkerImpl, "uart_event_task", 2048, NULL, 12, NULL);
}

size_t UART::write(uint8_t c)
{
    uart_write_bytes(uart_port, (const char *)c, sizeof(c));
    return 1;
}
