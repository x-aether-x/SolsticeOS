#include "console.h"
#include "utils.h"
 
#define SSFN_CONSOLEBITMAP_TRUECOLOR
#define SSFN_NO_CPP_STD_STRING
#define SSFN_memset memset
#define SSFN_realloc(p, n) (0)
#define SSFN_free(p) ((void)0)
#define SSFN_IMPLEMENTATION
#include "ssfn.h"
#include "utils.h"
 
static struct console_buffer g_console;
static int g_font_loaded = 0;
extern "C" unsigned char _binary_build_FreeSans_sfn_start[];

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

#define CONSOLE_BG (vga_palette[0])

void console_set_color(int fg, int bg) {
    ssfn_dst.fg = vga_palette[fg & 0x0F];
    ssfn_dst.bg = vga_palette[bg & 0x0F];
}
static void console_fill_rows(uint32_t y0, uint32_t rows, uint32_t color) {
    if (!g_console.ptr) return;
    for (uint32_t r = 0; r < rows; r++) {
        uint32_t* row = (uint32_t*)(g_console.ptr + ((uint64_t)(y0 + r) * g_console.pitch));
        for (uint32_t x = 0; x < g_console.pitch / 4; x++) row[x] = color;
    }
}


const int FONT_HEIGHT = 16;
const int FONT_WIDTH = 8;

void console_init(uint8_t* fb, uint32_t width, uint32_t height, uint32_t pitch) {
    g_console.ptr = fb;
    g_console.width = width;
    g_console.height = height;
    g_console.pitch = pitch;
    g_console.x = 0; g_console.y = 0;

    ssfn_dst.ptr = fb;
    ssfn_dst.w = width; ssfn_dst.h = height; ssfn_dst.p = pitch;
    ssfn_dst.fg = 0xFFFFFFFF; ssfn_dst.bg = 0x00000000;

    ssfn_src = (ssfn_font_t*)_binary_build_FreeSans_sfn_start; 
    g_font_loaded = 1;
    console_clear();
}

void console_clear(void) {
    console_fill_rows(0, g_console.height, CONSOLE_BG);
    g_console.x = 0; g_console.y = 0;
}


void console_backspace(void) {
    if (!g_console.ptr) return;
 
    if (g_console.x >= FONT_WIDTH) {
        g_console.x -= FONT_WIDTH;
    } else if (g_console.y >= FONT_HEIGHT) {
        g_console.y -= FONT_HEIGHT;
        g_console.x = (g_console.width / FONT_WIDTH - 1) * FONT_WIDTH;
    } else {
        return;
    }
 
    for (int row = 0; row < FONT_HEIGHT; row++) {
        uint32_t* pixel_row = (uint32_t*)((uint8_t*)g_console.ptr + 
                                       ((g_console.y + row) * g_console.pitch) + 
                                       (g_console.x * sizeof(uint32_t)));
        for (int i = 0; i < FONT_WIDTH; i++) {
            pixel_row[i] = CONSOLE_BG;
        }
    }
}

void console_scroll(void) {
    if (!g_console.ptr) return;
 
    // shift up
    size_t scroll_size = (g_console.height - FONT_HEIGHT) * g_console.pitch;
    memmove(g_console.ptr, (uint8_t*)g_console.ptr + (FONT_HEIGHT * g_console.pitch), scroll_size);
 
    // clear bottom
    console_fill_rows(g_console.height - FONT_HEIGHT, FONT_HEIGHT, CONSOLE_BG);
 
    // reset y
    g_console.y = g_console.height - FONT_HEIGHT;
    g_console.x = 0; 
 
    ssfn_dst.x = g_console.x;
    ssfn_dst.y = g_console.y;
}


void console_putc(char c) {
    if (!g_console.ptr || !g_font_loaded) return;
 
    if (c == '\n') {
        g_console.x = 0;
        g_console.y += FONT_HEIGHT;
    } else {
        ssfn_dst.x = g_console.x;
        ssfn_dst.y = g_console.y;
        ssfn_putc((uint32_t)(unsigned char)c);
        g_console.x = ssfn_dst.x;
        g_console.y = ssfn_dst.y;
 
        if (g_console.x + FONT_WIDTH > g_console.width) {
            g_console.x = 0;
            g_console.y += FONT_HEIGHT;
        }
    }
 
    if (g_console.y + FONT_HEIGHT > g_console.height) {
        console_scroll();
    }
}

void console_puts(const char* s) { for (; *s; s++) console_putc(*s); }