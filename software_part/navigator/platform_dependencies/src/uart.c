#include "uart.h"
#include "esp_log.h"
#include "freertos/queue.h"
#include <string.h>

static const char *TAG = "UART_GPS";

// Хендл UHCI контролера
static uhci_controller_handle_t uhci_ctrl;

// Буфер прийому
uint8_t rx_buffer[GPS_RX_BUF_SIZE];

// Черга подій між ISR і таском
QueueHandle_t uart_gps_queue;

// Контекст який передається в callback
typedef struct {
    QueueHandle_t queue;
    uint8_t      *data;
    size_t        size;
} gps_uhci_ctx_t;

static gps_uhci_ctx_t gps_ctx;

// ISR callback — викликається з переривання
// ВАЖЛИВО: тільки FromISR функції FreeRTOS!
static IRAM_ATTR bool gps_rx_callback(
    uhci_controller_handle_t ctrl,
    const uhci_rx_event_data_t *edata,
    void *user_ctx)
{
    gps_uhci_ctx_t *ctx = (gps_uhci_ctx_t *)user_ctx;
    BaseType_t higher_priority_woken = pdFALSE;

    gps_event_t evt;

    if (edata->flags.totally_received) {
        // Повний пакет отримано
        ctx->size = edata->recv_size;
        memcpy(ctx->data, edata->data, edata->recv_size);
        
        // Забезпечуємо нуль-термінатор для рядка, щоб ESP_LOGI(%s) працював коректно
        if (edata->recv_size < GPS_RX_BUF_SIZE) {
            ctx->data[edata->recv_size] = '\0';
        }
        
        evt = GPS_EVT_DATA_READY;
    } else {
        // Частковий прийом — продовжуємо
        evt = GPS_EVT_PARTIAL_DATA;
    }

    xQueueSendFromISR(ctx->queue, &evt, &higher_priority_woken);
    return higher_priority_woken == pdTRUE;
}

void uart_gps_initialize(void) {
    // Черга подій
    uart_gps_queue = xQueueCreate(10, sizeof(gps_event_t));
    if (uart_gps_queue == NULL) {
        ESP_LOGE(TAG, "Queue create failed");
        return;
    }

    // Налаштування UART
    uart_config_t uart_cfg = {
        .baud_rate  = GPS_UART_BAUD_RATE,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_param_config(GPS_UART_PORT, &uart_cfg));
    ESP_ERROR_CHECK(uart_set_pin(
        GPS_UART_PORT,
        GPS_UART_TX_PIN,
        GPS_UART_RX_PIN,
        UART_PIN_NO_CHANGE,
        UART_PIN_NO_CHANGE
    ));

    // Налаштування UHCI (DMA контролер)
    uhci_controller_config_t uhci_cfg = {
        .uart_port               = GPS_UART_PORT,
        .tx_trans_queue_depth    = GPS_TX_QUEUE_DEPTH,
        .max_receive_internal_mem = GPS_RX_BUF_SIZE,
        .max_transmit_size       = GPS_RX_BUF_SIZE,
        .dma_burst_size          = GPS_DMA_BURST_SIZE,
        // idle_eof: завершити прийом коли UART мовчить
        // ідеально для NMEA — рядки розділені паузами
        .rx_eof_flags.idle_eof   = 1,
    };

    ESP_ERROR_CHECK(uhci_new_controller(&uhci_cfg, &uhci_ctrl));

    // Реєструємо callback
    gps_ctx.queue = uart_gps_queue;
    gps_ctx.data  = rx_buffer;
    gps_ctx.size  = 0;

    uhci_event_callbacks_t cbs = {
        .on_rx_trans_event = gps_rx_callback,
    };
    ESP_ERROR_CHECK(uhci_register_event_callbacks(uhci_ctrl, &cbs, &gps_ctx));

    ESP_LOGI(TAG, "GPS UART initialized");
}

void uart_gps_start_receive(void) {
    // Запускаємо прийом — неблокуючий виклик
    // DMA сам заповнить rx_buffer коли прийдуть дані
    ESP_ERROR_CHECK(uhci_receive(uhci_ctrl, rx_buffer, GPS_RX_BUF_SIZE));
    ESP_LOGI(TAG, "Receiving started");
}

void uart_gps_deinitialize(void) {
    ESP_ERROR_CHECK(uhci_del_controller(uhci_ctrl));
    vQueueDelete(uart_gps_queue);
    ESP_LOGI(TAG, "GPS UART deinitialized");
}