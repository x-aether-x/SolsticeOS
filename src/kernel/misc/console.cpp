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

int console_draw_glyph(uint32_t* target, int pitch_bytes, int x, int y,
                          char c, uint32_t fg, uint32_t bg) {
    // save whatever the console was using
    uint8_t* saved_ptr   = ssfn_dst.ptr;
    int      saved_p     = ssfn_dst.p;
    int      saved_w     = ssfn_dst.w;
    int      saved_h     = ssfn_dst.h;
    int      saved_x     = ssfn_dst.x;
    int      saved_y     = ssfn_dst.y;
    uint32_t saved_fg    = ssfn_dst.fg;
    uint32_t saved_bg    = ssfn_dst.bg;

    ssfn_dst.ptr = (uint8_t*)target;
    ssfn_dst.p   = pitch_bytes;
    ssfn_dst.w   = g_console.width;   // clip bounds = screen dims
    ssfn_dst.h   = g_console.height;
    ssfn_dst.x   = x;
    ssfn_dst.y   = y;
    ssfn_dst.fg  = fg;
    ssfn_dst.bg  = bg;

    ssfn_putc((uint32_t)(unsigned char)c);

    int advance = ssfn_dst.x - x;   // how far the glyph moved the cursor

    ssfn_dst.ptr = saved_ptr;
    ssfn_dst.p   = saved_p;
    ssfn_dst.w   = saved_w;
    ssfn_dst.h   = saved_h;
    ssfn_dst.x   = saved_x;
    ssfn_dst.y   = saved_y;
    ssfn_dst.fg  = saved_fg;
    ssfn_dst.bg  = saved_bg;

    return advance > 0 ? advance : 8;  // fall back if the font didn't advance
}