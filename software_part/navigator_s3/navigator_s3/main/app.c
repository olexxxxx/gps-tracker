#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"

#define BUTTON_PIN GPIO_NUM_39
#define STATE_PIN  GPIO_NUM_40

typedef enum{
    reset = 0,
    set,
}state;

uint8_t state_pin = reset;
static QueueHandle_t button_queue;

static void IRAM_ATTR button_isr(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
}

int main(void){
    button_queue = xQueueCreate(10, sizeof(uint32_t));
    /*налаштування gpio*/
    gpio_config_t button_pin_conf = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),       
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE
    };

        gpio_config_t state_pin_conf = {
        .pin_bit_mask = (1ULL << STATE_PIN),       
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&button_pin_conf);
    gpio_config(&state_pin_conf);

    /*обробка натискання*/
    gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
    gpio_isr_handler_add(BUTTON_PIN, button_isr, NULL);
    /*запуск таймера*/
    /*рахує поки натиснута кнопка*/
    /*якщо дораховує - вмикається/вимикається система*/
    /*інакше - нічого не відбувається*/
}
