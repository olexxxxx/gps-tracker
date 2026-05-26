#ifndef __GPS_H
#define __GPS_H

#include <stdbool.h>
#include "uart.h"

// Структура для зберігання свіжих даних
typedef struct {
    float latitude;
    float longitude;
    int satellites;
    bool has_fix;
} gps_data_t;

// extern каже компілятору: "Ця змінна десь існує, дозволь іншим файлам її читати"
// volatile каже: "Ця змінна оновлюється з іншого таску, не кешуй її"
extern volatile gps_data_t current_gps_data;

void gps_task(void *pvParameters);

#endif //__GPS_H