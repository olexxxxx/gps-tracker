#include "gps.h"
#include "uart.h"
#include "esp_log.h"
#include "minmea.h"
#include <string.h>

static const char *TAG = "GPS";

// Реально створюємо цю змінну в пам'яті
volatile gps_data_t current_gps_data = {
    .latitude = 0.0,
    .longitude = 0.0,
    .satellites = 0,
    .has_fix = false
};

void gps_task(void *pvParameters) {
    gps_event_t evt;
    uart_gps_start_receive();

    while (1) {
        if (xQueueReceive(uart_gps_queue, &evt, portMAX_DELAY) == pdTRUE) {
            if (evt == GPS_EVT_DATA_READY) {
                
                char *line = strtok((char *)rx_buffer, "\r\n");
                
                while (line != NULL) {
                    switch (minmea_sentence_id(line, false)) {
                        
                        case MINMEA_SENTENCE_RMC: {
                            struct minmea_sentence_rmc frame;
                            if (minmea_parse_rmc(&frame, line)) {
                                current_gps_data.has_fix = frame.valid; // Оновлюємо статус
                                if (frame.valid) {
                                    // Оновлюємо координати
                                    current_gps_data.latitude = minmea_tocoord(&frame.latitude);
                                    current_gps_data.longitude = minmea_tocoord(&frame.longitude);
                                }
                            }
                            break;
                        }
                        
                        case MINMEA_SENTENCE_GGA: {
                            struct minmea_sentence_gga frame;
                            if (minmea_parse_gga(&frame, line)) {
                                if (frame.fix_quality > 0) {
                                    // Оновлюємо супутники
                                    current_gps_data.satellites = frame.satellites_tracked;
                                }
                            }
                            break;
                        }
                        
                        default:
                            break;
                    }
                    line = strtok(NULL, "\r\n");
                }
                uart_gps_start_receive();
            }
        }
    }
}