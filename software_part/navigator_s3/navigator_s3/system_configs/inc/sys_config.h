#ifndef SYS_CONFIG_H
#define SYS_CONFIG_H


//якщо треба буде щось швидко перевірити, то краще мати можливість просто заживити плату умовно з лабораторного БЖ, і не ініціалізувати систему увімкнення, а одразу налаштувати периферію
#define ALT_POWER_MODE 1

#include "freertos/FreeRTOS.h"
#include <stdbool.h>
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "power_boot.h"
#include "led_strip.h"

void initialize_power_boot(void);
void initialize_power_boot_peripherals(void);


/**
 * 
 * @brief Ініціалізація пристрою без використання системи живлення
 * 
 */

void initialize_system_without_power_boot(void);


#endif