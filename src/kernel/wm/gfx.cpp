#include "memory.h"
#include <stddef.h>
#include "gfx.h"
#include "console.h"
#include "utils.h"
#include "printf.h"
#include "cursor.h"

static uint32_t* backbuffer = nullptr;
static uint32_t* framebuffer = nullptr;
static int fb_width = 0, fb_height = 0;
static int fb_pitch = 0; // in bytes

void gfx_init(FramebufferInfo* fb) {
    fb_width = fb->Width;
    fb_height = fb->Height;
    fb_pitch = fb->Pitch;
    framebuffer = (uint32_t*)fb->BaseAddress;
    
    uint64_t backbuf_size = (uint64_t)fb_height * fb_pitch;
    uint64_t pages_needed = (backbuf_size + 4095) / 4096;
    
    (void)pages_needed;
    backbuffer = (uint32_t*)0x1000000; 
    
    uint8_t* ptr = (uint8_t*)backbuffer;
    for (uint64_t i = 0; i < backbuf_size; i++) {
        ptr[i] = 0;
    }
}

void gfx_fill_rect(int x, int y, int w, int h, uint32_t color) {
    // cut off at screen boundary
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > fb_width) w = fb_width - x;
    if (y + h > fb_height) h = fb_height - y;
    if (w <= 0 || h <= 0) return;
    
    for (int row = 0; row < h; row++) {
        uint32_t* dst = backbuffer + (y + row) * (fb_pitch / 4) + x;
        for (int col = 0; col < w; col++) {
            dst[col] = color;
        }
    }
}

void gfx_draw_rect(int x, int y, int w, int h, int thickness, uint32_t color) {
    gfx_fill_rect(x, y, w, thickness, color); // top
    gfx_fill_rect(x, y + h - thickness, w, thickness, color); // bottom
    gfx_fill_rect(x, y, thickness, h, color); // left
    gfx_fill_rect(x + w - thickness, y, thickness, h, color); // right
}

void gfx_blit(int dst_x, int dst_y, uint32_t* src, int w, int h) {
    // cioues src buffer into backbuffer
    if (dst_x < 0) { w += dst_x; src += -dst_x; dst_x = 0; }
    if (dst_y < 0) { h += dst_y; src += (-dst_y) * w; dst_y = 0; }
    if (dst_x + w > fb_width) w = fb_width - dst_x;
    if (dst_y + h > fb_height) h = fb_height - dst_y;
    if (w <= 0 || h <= 0) return;
    
    for (int row = 0; row < h; row++) {
        uint32_t* dst = backbuffer + (dst_y + row) * (fb_pitch / 4) + dst_x;
        uint32_t* s = src + row * w;
        for (int col = 0; col < w; col++) {
            dst[col] = s[col];
        }
    }
}

void gfx_draw_char(int x, int y, char c, uint32_t fg, uint32_t bg) {
    console_draw_glyph(backbuffer, fb_pitch, x, y, c, fg, bg);
}

void gfx_draw_string(int x, int y, const char* str, uint32_t fg, uint32_t bg) {
    int cur_x = x;
    for (; *str; str++) {
        cur_x += console_draw_glyph(backbuffer, fb_pitch, cur_x, y, *str, fg, bg);
    }
}

uint32_t* gfx_get_backbuffer() {
    return backbuffer;
}

int gfx_get_width() {
    return fb_width;
}

int gfx_get_height() {
    return fb_height;
}

void gfx_clear(uint32_t color) {
    for (int y = 0; y < fb_height; y++) {
        uint32_t* row = backbuffer + y * (fb_pitch / 4);
        for (int x = 0; x < fb_width; x++) row[x] = color;
    }
}

void gfx_present() {
    uint8_t* src = (uint8_t*)backbuffer;
    uint8_t* dst = (uint8_t*)framebuffer;
    for (int row = 0; row < fb_height; row++) {
        memcpy(dst, src, fb_pitch);
        src += fb_pitch;
        dst += fb_pitch;
    }
}

void gfx_draw_cursor() {
    int mx = mouse_x, my = mouse_y;
    // draws a trianglish cursor shape
    for (int r = 0; r < 13; r++) {
        int len = 13 - r;
        gfx_fill_rect(mx - 1, my + r - 1, len + 2, 1, 0xFF000000);
    }
    for (int r = 0; r < 12; r++) {
        int len = 12 - r;
        gfx_fill_rect(mx, my + r, len, 1, 0xFFFFFFFF);
    }
}