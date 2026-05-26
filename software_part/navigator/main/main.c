#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "spi.h"
#include "system_initialize.h"
#include "ili_9488.h"
#include "esp_log.h"
#include "uart.h"
#include "gps.h"
#include <stdio.h> // Потрібно для snprintf

static const char *TAG = "MAIN";

void app_main(void) {
    ESP_LOGI(TAG, "Starting...");
    
    // Ініціалізація заліза
    uart_gps_initialize();
    initialize_peripheral();
    initialize_display();
    
    fill_screen(0, 0, 0); 
    draw_string(15, 15, "GPS MONITOR", 255, 255, 255, 0, 0, 0);
    
    // Запускаємо фоновий таск парсингу
    xTaskCreate(gps_task, "gps_task", 4096, NULL, 5, NULL);
    
    char display_buf[40];

    // Головний цикл UI
    while (1) {
        // Читаємо дані з глобальної змінної і малюємо
        if (current_gps_data.has_fix) {
            snprintf(display_buf, sizeof(display_buf), "LAT: %.5f   ", current_gps_data.latitude);
            draw_string(15, 40, display_buf, 0, 255, 0, 0, 0, 0);

            snprintf(display_buf, sizeof(display_buf), "LON: %.5f   ", current_gps_data.longitude);
            draw_string(15, 55, display_buf, 0, 255, 0, 0, 0, 0);
        } else {
            draw_string(15, 40, "WAITING FOR FIX...      ", 255, 0, 0, 0, 0, 0);
            draw_string(15, 55, "                        ", 0, 0, 0, 0, 0, 0); // Затирання
        }

        snprintf(display_buf, sizeof(display_buf), "SATS: %d      ", current_gps_data.satellites);
        draw_string(15, 75, display_buf, 255, 255, 0, 0, 0, 0);

        // Оновлюємо екран раз на секунду. 
        // Швидше і не треба, GPS все одно частіше не шле
        vTaskDelay(pdMS_TO_TICKS(1000)); 
    }
}