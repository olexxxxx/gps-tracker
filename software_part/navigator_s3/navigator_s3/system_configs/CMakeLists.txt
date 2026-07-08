idf_component_register(
    SRCS "src/sys_config.c"
    INCLUDE_DIRS "inc"
    REQUIRES 
        driver 
        esp_timer 
        freertos 
        power_boot
        espressif__led_strip
)