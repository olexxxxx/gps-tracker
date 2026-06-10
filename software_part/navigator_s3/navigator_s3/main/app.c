#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <freertos/semphr.h>
#include <freertos/timers.h>
#include <esp_timer.h>
#include "esp_log.h"
#include "driver/gpio.h"
#include "led_strip.h"
#include <stdio.h>
#include <stdbool.h>
#include "sdkconfig.h"

/*========DEFINES========*/
#define BUTTON_PIN            GPIO_NUM_39
#define STATE_PIN             GPIO_NUM_40
#define BLINK_GPIO            GPIO_NUM_48
#define DEBOUNCE_DELAY_US     100000ULL
#define INITIALIZE_TIMEOUT_MS 3000ULL

typedef enum{
    LOW = 0,
    HIGH,
}state_t;


/*==========INTERNAL FUNCTION PROTOTYPES============*/
static void configure_power_boot_peripherals(void);
static void initialize_system_task(void *arg);
static void deinitialize_system_task(void *arg);

/*==========INTERNAL VARIABLES======================*/
static volatile uint64_t     last_isr_time = 0;
SemaphoreHandle_t            button_semaphore = NULL;
TimerHandle_t                power_boot_timer;
static volatile bool         system_is_initialized = false;


static void IRAM_ATTR button_isr(void* arg){
    uint64_t now = esp_timer_get_time(); 
    if (now - last_isr_time >= DEBOUNCE_DELAY_US){
        last_isr_time = now;
        BaseType_t higher_priority_task_woken = pdFALSE;
        xSemaphoreGiveFromISR(button_semaphore, &higher_priority_task_woken);
        portYIELD_FROM_ISR(higher_priority_task_woken);
    }
}

static void power_boot_timer_callback(TimerHandle_t xTimer){
    if(gpio_get_level(BUTTON_PIN) == LOW){
        if(!system_is_initialized){
                xTaskCreate(
                    initialize_system_task, 
                    "Initialize System Task", 
                    2048, 
                    NULL, 
                    10, 
                    NULL);
        }
        else{
                xTaskCreate(
                    deinitialize_system_task, 
                    "Deinitialize System Task", 
                    2048, 
                    NULL, 
                    10, 
                    NULL);
        }            
    }
    else{
        if(!system_is_initialized){
        /*Якщо кнопка трималась недостатньо довго в низькому стані - вимикаємо пін стану, таким чином зупиняємо ініціалізацію системи*/
            gpio_set_level(STATE_PIN, LOW);
        }
    }
}


static void power_boot_task(void *arg){
    while(1){
        if(xSemaphoreTake(button_semaphore, portMAX_DELAY) == pdTRUE){
            xTimerStart(power_boot_timer, 0);
        }
    }
}


void app_main(void){
    configure_power_boot_peripherals();
    gpio_set_level(STATE_PIN, HIGH);
    button_semaphore = xSemaphoreCreateBinary();
    power_boot_timer = xTimerCreate("Power Boot Timer", pdMS_TO_TICKS(INITIALIZE_TIMEOUT_MS), false, 0, power_boot_timer_callback);
    gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
    gpio_isr_handler_add(BUTTON_PIN, button_isr, NULL);
    xTaskCreate(
        power_boot_task, 
        "Power Boot Task", 
        2048, 
        NULL, 
        10, 
        NULL);
    while(1){
    vTaskDelay(portMAX_DELAY);
    }
}


/*================INTERNAL FUNCTIONS===================*/


/**
 * @brief Ініціалізація пінів для кнопки та state-піна, щоб
 * можна було керувати подачею живлення для увімкнення/вимкнення пристрою
 */
static void configure_power_boot_peripherals(void){
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

/** 
*
* @brief Ініціалізація системи.
* Активується, коли кнопка при старті затиснута довше ніж 3 секунди
*  
*/
static void initialize_system_task(void *arg){
    /*в якості тимчасового коду тут просто буде налагодження стандартного RGB з прикладу для ESP32s3, потім буде реалізована нормальна ініціалізація системи */

    led_strip_handle_t led_strip;
    led_strip_config_t strip_config = {
        .strip_gpio_num = BLINK_GPIO, 
        .max_leds = 1,                
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000,
        .flags.with_dma = false,           
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    led_strip_clear(led_strip);
    system_is_initialized = true;
    vTaskDelete(NULL); 
}

/** 
*
* @brief Деініціалізація системи.
* Активується, коли кнопка при старті затиснута довше ніж 3 секунди
*  
*/
static void deinitialize_system_task(void *arg){
    /*деініціалізація системи, опускаємо пін стану*/
    gpio_set_level(STATE_PIN, LOW);
    system_is_initialized = false;
    vTaskDelete(NULL); 
}
