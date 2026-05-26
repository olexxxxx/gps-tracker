#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stddef.h>
#include "driver/uart.h"
#include "driver/uhci.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

// Піни UART для GPS
#define GPS_UART_PORT       UART_NUM_1
#define GPS_UART_TX_PIN     21
#define GPS_UART_RX_PIN     20
#define GPS_UART_BAUD_RATE  9600  // стандарт для NEO-6M

// Розміри буферів
#define GPS_RX_BUF_SIZE     512   // максимальний NMEA рядок ~82 байти
#define GPS_TX_QUEUE_DEPTH  10
#define GPS_DMA_BURST_SIZE  32

// Перенесено сюди, щоб gps.c "бачив" ці події
typedef enum {
    GPS_EVT_DATA_READY,
    GPS_EVT_PARTIAL_DATA,
    GPS_EVT_ERROR
} gps_event_t;

// Черга для передачі даних з ISR в таск
extern QueueHandle_t uart_gps_queue;

// Глобальний буфер (щоб його міг читати gps.c)
extern uint8_t rx_buffer[GPS_RX_BUF_SIZE];

// Ініціалізація UART з DMA для GPS
void uart_gps_initialize(void);

// Запустити прийом (неблокуючий)
void uart_gps_start_receive(void);

// Зупинити і вивільнити ресурси
void uart_gps_deinitialize(void);

#endif // UART_H