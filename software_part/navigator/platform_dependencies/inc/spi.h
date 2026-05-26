#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define SPI_PIN_SCK     4
#define SPI_PIN_MOSI    6
#define SPI_PIN_MISO    5
#define SPI_PIN_CS      7



#define SPI_CLOCK_HZ    (100 * 100 * 1000)

extern spi_device_handle_t spi_device;

void spi_initialize_bus(void);
void spi_send_byte(uint8_t data);
void spi_send_buffer(const uint8_t *data, uint32_t length);
void spi_set_cs(uint8_t level);

#endif // SPI_H