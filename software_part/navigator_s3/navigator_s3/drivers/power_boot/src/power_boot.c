#include "power_boot.h"


void configure_power_boot_peripherals(void){
        gpio_config_t button_pin_conf = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),       
        .mode = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_NEGEDGE
    };
    gpio_config(&button_pin_conf);

        gpio_config_t state_pin_conf = {
        .pin_bit_mask = (1ULL << STATE_PIN),       
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE
    };
    gpio_config(&state_pin_conf);
}


