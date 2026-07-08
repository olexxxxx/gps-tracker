#include <stdio.h>
#include <stdbool.h>
#include "app.h"






void app_main(void){
    #if ALT_POWER_MODE
    initialize_system_without_power_boot();
    #else
    initialize_power_boot();
    #endif
    while(1){
    vTaskDelay(portMAX_DELAY);
    }
}


