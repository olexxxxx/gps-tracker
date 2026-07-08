#ifndef POWER_BOOT_H
#define POWER_BOOT_H

/*=================INCLUDES==================*/
#include <esp_timer.h>
#include "esp_log.h"
#include "driver/gpio.h"



/*===================DEFINES=================*/
#define BUTTON_PIN            GPIO_NUM_39
#define STATE_PIN             GPIO_NUM_40
#define BLINK_GPIO            GPIO_NUM_48
#define DEBOUNCE_DELAY_US     100000ULL
#define INITIALIZE_TIMEOUT_MS 3000ULL

typedef enum{
    POWER_LOW = 0,
    POWER_HIGH,
}state_t;


/**
 * @brief Ініціалізація пінів для кнопки та state-піна, щоб
 * можна було керувати подачею живлення для увімкнення/вимкнення пристрою
 */
void configure_power_boot_peripherals(void);


#endif