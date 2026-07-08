#include "console.h"
#include "utils.h"

#define SSFN_CONSOLEBITMAP_TRUECOLOR
#define SSFN_NO_CPP_STD_STRING
#define SSFN_memset memset
#define SSFN_realloc(p, n) (0)
#define SSFN_free(p) ((void)0)
#define SSFN_IMPLEMENTATION
#include "ssfn.h"

static struct console_buffer g_console;
static int g_font_loaded = 0;
extern "C" unsigned char _binary_build_FreeSans_sfn_start[];

void console_init(uint8_t* fb, uint32_t width, uint32_t height, uint32_t pitch) {
    g_console.ptr = fb;
    g_console.width = width;
    g_console.height = height;
    g_console.pitch = pitch;
    g_console.x = 0; g_console.y = 0;

    ssfn_dst.ptr = fb;
    ssfn_dst.w = width; ssfn_dst.h = height; ssfn_dst.p = pitch;
    ssfn_dst.fg = 0xFFFFFFFF; ssfn_dst.bg = 0x00000000;

    ssfn_load(NULL, _binary_build_FreeSans_sfn_start);
    g_font_loaded = 1;
    console_clear();
}

void console_clear(void) {
    if (g_console.ptr) memset(g_console.ptr, 0, g_console.height * g_console.pitch);
    g_console.x = 0; g_console.y = 0;
}

void console_putc(char c) {
    if (!g_console.ptr || !g_font_loaded) return;
    if (c == '\n') { g_console.x = 0; g_console.y += 16; return; }
    
    ssfn_dst.x = g_console.x; ssfn_dst.y = g_console.y;
    ssfn_putc((uint32_t)(unsigned char)c);
    g_console.x = ssfn_dst.x; g_console.y = ssfn_dst.y;
}

void console_puts(const char* s) { for (; *s; s++) console_putc(*s); }