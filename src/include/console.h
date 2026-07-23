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

static const uint32_t vga_palette[16] = {
    0xFF16091F, // 0  deep plum (background)
    0xFF4C2A85, // 1  deep violet
    0xFF7FE8A2, // 2  mint green
    0xFF8AD8E8, // 3  pastel cyan
    0xFFFF8FA3, // 4  soft rose (errors)
    0xFF9D7BEA, // 5  purple
    0xFFB894F6, // 6  mauve
    0xFFD9CCF2, // 7  lavender grey
    0xFF6E5A8E, // 8  muted purple
    0xFFC7A6FF, // 9  light violet
    0xFFA8F0C0, // 10 soft green
    0xFFF3B8E8, // 11 pastel pink
    0xFFFF79C6, // 12 hot pink
    0xFFFF9EDB, // 13 light pink
    0xFFF6E3A1, // 14 soft gold
    0xFFF5EFFF  // 15 lavender white
};

void console_init(uint8_t* fb, uint32_t width, uint32_t height, uint32_t pitch);
void console_clear(void);
void console_set_color(int fg, int bg);
void console_backspace();
void console_putc(char c);
void console_puts(const char* s);
int console_draw_glyph(uint32_t* target, int pitch_bytes, int x, int y, char c, uint32_t fg, uint32_t bg);