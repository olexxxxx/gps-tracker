#include "spi.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include <string.h>

static const char *TAG = "SPI";

// Хендл SPI пристрою — використовується в send функціях
spi_device_handle_t spi_device;

void spi_initialize_bus(void) {
    // Конфігурація шини SPI
    spi_bus_config_t bus_config = {
        .mosi_io_num     = SPI_PIN_MOSI,
        .miso_io_num     = SPI_PIN_MISO,
        .sclk_io_num     = SPI_PIN_SCK,
        .quadwp_io_num   = -1,   // не використовується
        .quadhd_io_num   = -1,   // не використовується
        .max_transfer_sz = 4092 // максимальний розмір DMA транзакції
    };

    // Ініціалізація шини з DMA
    // SPI2_HOST — єдина доступна SPI шина на ESP32-C3
    // SPI_DMA_CH_AUTO — ESP-IDF сам вибирає DMA канал
    esp_err_t ret = spi_bus_initialize(SPI2_HOST,
                                       &bus_config,
                                       SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI bus init failed: %s", esp_err_to_name(ret));
        return;
    }

    // Конфігурація пристрою на шині
    spi_device_interface_config_t dev_config = {
        .clock_speed_hz  = SPI_CLOCK_HZ,
        .mode            = 0,          // CPOL=0, CPHA=0
        .spics_io_num    = SPI_PIN_CS, // CS керується апаратно
        .queue_size      = 8,          // глибина черги транзакцій
        .flags           = 0
    };

    ret = spi_bus_add_device(SPI2_HOST, &dev_config, &spi_device);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI device add failed: %s", esp_err_to_name(ret));
        return;
    }

    ESP_LOGI(TAG, "SPI initialized");
}

void spi_send_byte(uint8_t data) {
    spi_transaction_t transaction = {
        .flags = SPI_TRANS_USE_TXDATA, // Беремо дані прямо зі структури
        .length = 8,
        .tx_data = {data}              // Кладемо байт сюди
    };
    spi_device_transmit(spi_device, &transaction);
}

void spi_send_buffer(const uint8_t *data, uint32_t length) {
    if (length == 0) return;

    // Виділяємо DMA-capable буфер
    // Звичайний стековий масив може бути поза зоною DMA
    uint8_t *dma_buf = heap_caps_malloc(length, MALLOC_CAP_DMA);
    if (dma_buf == NULL) {
        ESP_LOGE(TAG, "DMA alloc failed");
        return;
    }
    memcpy(dma_buf, data, length);

    spi_transaction_t transaction = {
        .length    = length * 8,  // в бітах
        .tx_buffer = dma_buf,
        .rx_buffer = NULL
    };

    // Ставимо в чергу — неблокуючий виклик
    spi_device_queue_trans(spi_device, &transaction, portMAX_DELAY);

    // Чекаємо завершення і звільняємо буфер
    spi_transaction_t *result;
    spi_device_get_trans_result(spi_device, &result, portMAX_DELAY);

    heap_caps_free(dma_buf);
}

void spi_set_cs(uint8_t level) {
    gpio_set_level(SPI_PIN_CS, level);
}