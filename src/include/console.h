#pragma once
#include <stdint.h>
 
struct console_buffer {
    uint8_t* ptr;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t x;
    uint32_t y;
};
 
void console_init(uint8_t* fb, uint32_t width, uint32_t height, uint32_t pitch);
void console_clear(void);
void console_set_color(int fg, int bg);
void console_backspace();
void console_putc(char c);
void console_puts(const char* s);