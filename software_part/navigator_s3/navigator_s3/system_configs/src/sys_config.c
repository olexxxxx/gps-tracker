#include "sys_config.h"


/*==========INTERNAL FUNCTION PROTOTYPES============*/
static void initialize_system_task(void *arg);
static void deinitialize_system_task(void *arg);
static void start_led_blink(void);
static void configure_led(void);
static void blink_task(void *arg);

/*==========INTERNAL VARIABLES======================*/
static volatile uint64_t            last_isr_time = 0;
static SemaphoreHandle_t            button_semaphore = NULL;
static TimerHandle_t                power_boot_timer;
static volatile bool                system_is_initialized = false;
static led_strip_handle_t           led_strip;

/*================FUNCTION INITIALIZE==================*/

void initialize_power_boot(void){
    configure_power_boot_peripherals();
    gpio_set_level(STATE_PIN, POWER_HIGH);
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
}

void initialize_system_without_power_boot(void){
        xTaskCreate(
            initialize_system_task, 
            "Initialize System Task", 
            4096, 
            NULL, 
            10, 
            NULL);
}


/*================INTERNAL FUNCTIONS===================*/

/** 
*
* @brief Ініціалізація системи.
* Активується, коли кнопка при старті затиснута довше ніж 3 секунди
*  
*/
static void initialize_system_task(void *arg){
    /*в якості тимчасового коду тут просто буде налагодження стандартного RGB з прикладу для ESP32s3, потім буде реалізована нормальна ініціалізація системи */
    configure_led();
    /*код для I2C, SPI, UART, налаштування інтерфейсу, драйверів ітд*/
    system_is_initialized = true;
    xTaskCreate(blink_task, "LED Blink", 2048, NULL, 5, NULL);
    vTaskDelete(NULL); 
}


static void configure_led(void) {
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
}

static void blink_task(void *arg) {
    uint8_t colors[][3] = {
        {32, 0, 0},   // червоний
        {0, 32, 0},   // зелений
        {0, 0, 32},   // синій
    };
    int idx = 0;

    while (1) {
        led_strip_set_pixel(led_strip, 0, colors[idx][0], colors[idx][1], colors[idx][2]);
        led_strip_refresh(led_strip);
        idx = (idx + 1) % 3;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// Виклич це з initialize_system_task або звідки завгодно
static void start_led_blink(void) {
    configure_led();
    xTaskCreate(blink_task, "LED Blink Task", 2048, NULL, 5, NULL);
}


/** 
*
* @brief Деініціалізація системи.
* Активується, коли кнопка затиснута довше ніж 3 секунди, деактивує систему
*  
*/
static void deinitialize_system_task(void *arg){
    /*деініціалізація системи, опускаємо пін стану*/
    gpio_set_level(STATE_PIN, POWER_LOW);
    system_is_initialized = false;
    vTaskDelete(NULL); 
}



/*=============ISR HANDLERS AND CALLBACKS===========*/
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
    if(gpio_get_level(BUTTON_PIN) == POWER_LOW){
        if(!system_is_initialized){
                xTaskCreate(
                    initialize_system_task, 
                    "Initialize System Task", 
                    4096, 
                    NULL, 
                    10, 
                    NULL);
        }
        else{
                xTaskCreate(
                    deinitialize_system_task, 
                    "Deinitialize System Task", 
                    4096, 
                    NULL, 
                    10, 
                    NULL);
        }            
    }
    else{
        if(!system_is_initialized){
        /*Якщо кнопка трималась недостатньо довго в низькому стані - вимикаємо пін стану, таким чином зупиняємо ініціалізацію системи*/
            gpio_set_level(STATE_PIN, POWER_LOW);
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

