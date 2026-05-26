#ifndef __ILI_9488_H
#define __ILI_9488_H

#include <stdint.h>

#define SLEEP_OUT               0x11
#define NORMAL_DISPLAY_MODE_ON 	0x13
#define DISPLAY_ON		        0x29
#define SOFTWARE_RESET		    0x01
#define MEMORY_WRITE		    0x2C
#define PAGE_ADDRESS_SET	    0x2B
#define COLUMN_ADDRESS_SET	    0x2A
#define INTERFACE_PIXEL_FORMAT  0x3A
#define MEMORY_ACCESS_CONTROL   0x36
#define COLOR_INVERSION_OFF     0x20
#define COLOR_INVERSION_ON      0x21

#define DC_PIN                  3U
#define RST_PIN                 2U




void initialize_display();
void hardware_reset();
void set_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void fill_screen(uint8_t r, uint8_t g, uint8_t b);
int get_char_index(char c);
void draw_string(uint16_t x, uint16_t y, const char *str, uint8_t r, uint8_t g, uint8_t b, uint8_t bg_r, uint8_t bg_g, uint8_t bg_b);
void draw_char(uint16_t x, uint16_t y, char c, uint8_t r, uint8_t g, uint8_t b, uint8_t bg_r, uint8_t bg_g, uint8_t bg_b);


#endif // __ILI9488_H


