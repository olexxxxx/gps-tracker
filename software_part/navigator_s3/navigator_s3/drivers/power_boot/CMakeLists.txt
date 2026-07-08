idf_component_register(
    SRCS "src/power_boot.c"
    INCLUDE_DIRS "inc"
    REQUIRES esp_driver_gpio esp_timer
)