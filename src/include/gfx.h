#pragma once
#include <stdint.h>

struct FramebufferInfo;

void gfx_init(FramebufferInfo* fb);
void gfx_fill_rect(int x, int y, int w, int h, uint32_t color);
void gfx_draw_rect(int x, int y, int w, int h, int thickness, uint32_t color); // outline only
void gfx_blit(int dst_x, int dst_y, uint32_t* src, int w, int h);
void gfx_draw_char(int x, int y, char c, uint32_t fg, uint32_t bg);
void gfx_draw_string(int x, int y, const char* str, uint32_t fg, uint32_t bg);
uint32_t* gfx_get_backbuffer();
int gfx_get_width();
int gfx_get_height();
void gfx_present(); // copy backbuffer to framebuffer
void gfx_clear(uint32_t color); // fill backbuffer with a color
void gfx_draw_cursor();