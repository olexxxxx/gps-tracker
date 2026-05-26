#include "ili_9488.h"
#include "spi.h"
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include <stdint.h>

static void send_command_to_ili9488(uint8_t command);
static void send_data_to_ili9488(uint8_t data);

// Оновлений масив шрифту (тепер 39 символів)
const uint8_t mini_font[39][8] = {
    // Літери A-Z (Індекси 0-25)
    { 0x0C, 0x1E, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x00},   // A
    { 0x3F, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3F, 0x00},   // B
    { 0x3C, 0x66, 0x03, 0x03, 0x03, 0x66, 0x3C, 0x00},   // C
    { 0x1F, 0x36, 0x66, 0x66, 0x66, 0x36, 0x1F, 0x00},   // D
    { 0x7F, 0x46, 0x16, 0x1E, 0x16, 0x46, 0x7F, 0x00},   // E
    { 0x7F, 0x46, 0x16, 0x1E, 0x16, 0x06, 0x0F, 0x00},   // F
    { 0x3C, 0x66, 0x03, 0x03, 0x73, 0x66, 0x7C, 0x00},   // G
    { 0x33, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x33, 0x00},   // H
    { 0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // I
    { 0x78, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E, 0x00},   // J
    { 0x67, 0x66, 0x36, 0x1E, 0x36, 0x66, 0x67, 0x00},   // K
    { 0x0F, 0x06, 0x06, 0x06, 0x46, 0x66, 0x7F, 0x00},   // L
    { 0x63, 0x77, 0x7F, 0x7F, 0x6B, 0x63, 0x63, 0x00},   // M
    { 0x63, 0x67, 0x6F, 0x7B, 0x73, 0x63, 0x63, 0x00},   // N
    { 0x1C, 0x36, 0x63, 0x63, 0x63, 0x36, 0x1C, 0x00},   // O
    { 0x3F, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x0F, 0x00},   // P
    { 0x1E, 0x33, 0x33, 0x33, 0x3B, 0x1E, 0x38, 0x00},   // Q
    { 0x3F, 0x66, 0x66, 0x3E, 0x36, 0x66, 0x67, 0x00},   // R
    { 0x1E, 0x33, 0x07, 0x0E, 0x38, 0x33, 0x1E, 0x00},   // S
    { 0x3F, 0x2D, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // T
    { 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3F, 0x00},   // U
    { 0x33, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00},   // V
    { 0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00},   // W
    { 0x63, 0x63, 0x36, 0x1C, 0x1C, 0x36, 0x63, 0x00},   // X
    { 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x1E, 0x00},   // Y
    { 0x7F, 0x63, 0x31, 0x18, 0x4C, 0x66, 0x7F, 0x00},   // Z
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // 26: ' ' (Пробіл)
    
    // Нові символи (Індекси 27-38)
    { 0x18, 0x24, 0x42, 0x42, 0x42, 0x24, 0x18, 0x00},   // 27: '0' 
    { 0x10, 0x18, 0x14, 0x10, 0x10, 0x10, 0x7C, 0x00},   // 28: '1'
    { 0x38, 0x44, 0x40, 0x30, 0x08, 0x04, 0x7C, 0x00},   // 29: '2'
    { 0x38, 0x44, 0x40, 0x38, 0x40, 0x44, 0x38, 0x00},   // 30: '3'
    { 0x30, 0x28, 0x24, 0x22, 0x7E, 0x20, 0x20, 0x00},   // 31: '4'
    { 0x7C, 0x04, 0x3C, 0x40, 0x40, 0x44, 0x38, 0x00},   // 32: '5'
    { 0x38, 0x04, 0x02, 0x3A, 0x46, 0x44, 0x38, 0x00},   // 33: '6'
    { 0x7C, 0x40, 0x20, 0x10, 0x08, 0x08, 0x08, 0x00},   // 34: '7'
    { 0x38, 0x44, 0x44, 0x38, 0x44, 0x44, 0x38, 0x00},   // 35: '8' 
    { 0x38, 0x44, 0x42, 0x5C, 0x40, 0x20, 0x1C, 0x00},   // 36: '9'
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00},   // 37: '.' 
    { 0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x00}    // 38: ':'
};

// Оптимізований пошук індексу за допомогою ASCII математики
int get_char_index(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';             // A-Z
    if (c >= '0' && c <= '9') return c - '0' + 27;        // 0-9
    if (c == '.') return 37;                              // Крапка
    if (c == ':') return 38;                              // Двокрапка
    return 26;                                            // Все інше - пробіл
}

void initialize_display(){
    gpio_set_direction(RST_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(DC_PIN, GPIO_MODE_OUTPUT);

    hardware_reset();

    send_command_to_ili9488(SOFTWARE_RESET);
    vTaskDelay(pdMS_TO_TICKS(120));
    send_command_to_ili9488(SLEEP_OUT);
    vTaskDelay(pdMS_TO_TICKS(120));

// Встановлюємо орієнтацію та кольори ОДНІЄЮ командою
    send_command_to_ili9488(MEMORY_ACCESS_CONTROL);
    

    send_data_to_ili9488(0x08 | 0x20); // Тобто 0x28
    

    send_command_to_ili9488(COLOR_INVERSION_ON);
    send_command_to_ili9488(NORMAL_DISPLAY_MODE_ON);
    send_command_to_ili9488(DISPLAY_ON);
}

void hardware_reset() {
    gpio_set_level(RST_PIN, 0);  // тримаємо LOW
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(RST_PIN, 1);  // відпускаємо
    vTaskDelay(pdMS_TO_TICKS(120));
}

void set_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    // X
    send_command_to_ili9488(COLUMN_ADDRESS_SET);
    send_data_to_ili9488(x1 >> 8);    // SC MSB
    send_data_to_ili9488(x1 & 0xFF);  // SC LSB
    send_data_to_ili9488(x2 >> 8);    // EC MSB
    send_data_to_ili9488(x2 & 0xFF);  // EC LSB
    // Y
    send_command_to_ili9488(PAGE_ADDRESS_SET);
    send_data_to_ili9488(y1 >> 8);    // SP MSB
    send_data_to_ili9488(y1 & 0xFF);  // SP LSB
    send_data_to_ili9488(y2 >> 8);    // EP MSB
    send_data_to_ili9488(y2 & 0xFF);  // EP LSB
}


void fill_screen(uint8_t r, uint8_t g, uint8_t b) {
    set_window(0, 0, 319, 479);
    send_command_to_ili9488(MEMORY_WRITE);
    gpio_set_level(DC_PIN, 1); // data mode

    uint16_t width = 320;
    uint16_t height = 480;
    uint32_t line_size = width * 3; // 3 байти на піксель

    // Виділяємо пам'ять для одного рядка
    uint8_t *line_buffer = heap_caps_malloc(line_size, MALLOC_CAP_DMA);
    if (!line_buffer) return;

    // Заповнюємо рядок потрібним кольором
    for (int i = 0; i < width; i++) {
        line_buffer[i * 3 + 0] = r;
        line_buffer[i * 3 + 1] = g;
        line_buffer[i * 3 + 2] = b;
    }

    // Відправляємо цей рядок 480 разів (по висоті екрану)
    for (int i = 0; i < height; i++) {
        // Щоб не переписувати твою функцію spi_send_buffer, 
        // яка робить malloc всередині, краще тут напряму викликати транзакцію:
        spi_transaction_t transaction = {
            .length = line_size * 8,
            .tx_buffer = line_buffer
        };
        spi_device_transmit(spi_device, &transaction);
    }

    heap_caps_free(line_buffer);
}


// r, g, b - колір тексту
// bg_r, bg_g, bg_b - колір фону (щоб затирати старий текст)
void draw_char(uint16_t x, uint16_t y, char c, uint8_t r, uint8_t g, uint8_t b, uint8_t bg_r, uint8_t bg_g, uint8_t bg_b) {
    int char_index = get_char_index(c);
    
    // Розмір буфера: 8 пікселів ширина * 8 висота * 3 байти на колір
    uint32_t buf_size = 8 * 8 * 3;
    uint8_t *char_buf = heap_caps_malloc(buf_size, MALLOC_CAP_DMA);
    if (!char_buf) return;

    int ptr = 0;
    // Проходимось по 8 рядках символу
    for (int row = 0; row < 8; row++) {
        uint8_t line = mini_font[char_index][row];
        
        // Проходимось по 8 бітах (пікселях) у рядку зліва направо
        for (int col = 0; col < 8; col++) {
            // Перевіряємо, чи встановлений поточний біт
            if (line & (1 << col)) { 
                // Біт = 1, малюємо колір літери
                char_buf[ptr++] = r;
                char_buf[ptr++] = g;
                char_buf[ptr++] = b;
            } else { 
                // Біт = 0, малюємо колір фону
                char_buf[ptr++] = bg_r;
                char_buf[ptr++] = bg_g;
                char_buf[ptr++] = bg_b;
            }
        }
    }

    // Встановлюємо вікно рівно розміром 8x8 пікселів
    set_window(x, y, x + 7, y + 7);
    send_command_to_ili9488(MEMORY_WRITE);
    gpio_set_level(DC_PIN, 1); // data mode
    
    // Відправляємо сформовану матрицю літери за одну транзакцію
    spi_transaction_t transaction = {
        .length = buf_size * 8,
        .tx_buffer = char_buf
    };
    spi_device_transmit(spi_device, &transaction);

    heap_caps_free(char_buf);
}

void draw_string(uint16_t x, uint16_t y, const char *str, uint8_t r, uint8_t g, uint8_t b, uint8_t bg_r, uint8_t bg_g, uint8_t bg_b) {
    uint16_t curr_x = x;
    
    while (*str) {
        draw_char(curr_x, y, *str, r, g, b, bg_r, bg_g, bg_b);
        curr_x += 8; // Крок на ширину символу
        
        // Захист: якщо вилізли за правий край екрану (320 пікс) - переносимо рядок
        if (curr_x > (320 - 8)) {
            curr_x = x;
            y += 8; // Крок на висоту символу вниз
        }
        str++;
    }
}

//-----LOCAL FUNCTIONS----//

static void send_command_to_ili9488(uint8_t command){
    gpio_set_level(DC_PIN, 0);
    spi_send_byte(command);
}

static void send_data_to_ili9488(uint8_t data){
    gpio_set_level(DC_PIN, 1);
    spi_send_byte(data);
}