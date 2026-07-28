#include "cursor.h"
#include <stdint.h>
#include "wm.h"
#include "memory.h"
#include "utils.h"
#include "gfx.h"
#include "timer.h"

#define TITLEBAR_H 24
#define CURSOR_W 14
#define CURSOR_H 14
#define MAX_WINDOWS 8

Window windows[MAX_WINDOWS];
int window_count = 0;

Window* console_window = nullptr;

static volatile bool have_dirty = false;
static volatile int dx0, dy0, dx1, dy1;

static void add_dirty(int x, int y, int w, int h) {
    int x1 = x + w, y1 = y + h;
    if (!have_dirty) { dx0 = x; dy0 = y; dx1 = x1; dy1 = y1; have_dirty = true; }
    else {
        if (x  < dx0) dx0 = x;
        if (y  < dy0) dy0 = y;
        if (x1 > dx1) dx1 = x1;
        if (y1 > dy1) dy1 = y1;
    }
}

// whole screen clear
void wm_mark_dirty() { add_dirty(0, 0, gfx_get_width(), gfx_get_height()); }
void wm_mark_dirty_rect(int x, int y, int w, int h) { add_dirty(x, y, w, h); }

Window* wm_create_window(const char* title, int x, int y, int w, int h) {
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (!windows[i].in_use) {
            static uint64_t wm_heap_next = 0x1400000; // after the buffer
            
            windows[i].in_use = true;
            windows[i].x = x;
            windows[i].y = y;
            windows[i].width = w;
            windows[i].height = h;
            windows[i].buffer = (uint32_t*)wm_heap_next;
            wm_heap_next += (uint64_t)w * h * 4;
            for (int p = 0; p < w * h; p++) windows[i].buffer[p] = 0xFF16091F;

            int j = 0;
            while (title[j] != '\0' && j < 31) {
                windows[i].title[j] = title[j];
                j++;
            }
            windows[i].title[j] = '\0';

            windows[i].layer = window_count++;
            windows[i].is_dirty = true;

            return &windows[i];
        }
    }
    return NULL;
}

void wm_draw() {
    // list of windows sorted by layer
    int order[MAX_WINDOWS];
    int n = 0;
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (!windows[i].in_use) continue;
        int j = n++;
        while (j > 0 && windows[order[j - 1]].layer > windows[i].layer) {
            order[j] = order[j - 1];
            j--;
        }
        order[j] = i;
    }

    for (int k = 0; k < n; k++) {
        Window* win = &windows[order[k]];

        // title bar
        gfx_fill_rect(win->x, win->y - TITLEBAR_H, win->width, TITLEBAR_H, 0xFF4C2A85);
        gfx_draw_string(win->x + 6, win->y - TITLEBAR_H + 4, win->title, 0xFFF5EFFF, 0xFF4C2A85);
        // close button
        gfx_fill_rect(win->x + win->width - TITLEBAR_H + 4, win->y - TITLEBAR_H + 4,
                      TITLEBAR_H - 8, TITLEBAR_H - 8, 0xFFFF8FA3);
        // input area
        gfx_blit(win->x, win->y, win->buffer, win->width, win->height);
        // border
        gfx_draw_rect(win->x - 1, win->y - TITLEBAR_H - 1,
                      win->width + 2, win->height + TITLEBAR_H + 2, 1, 0xFF9D7BEA);
    }
}

void wm_task() {
    uint64_t last = 0;
    int last_mx = mouse_x, last_my = mouse_y;

    asm volatile("cli");
    gfx_reset_clip();
    gfx_clear(0xFF16091F);
    wm_draw();
    gfx_draw_cursor();
    gfx_present();
    asm volatile("sti");

    for (;;) {
        if (timer_ticks - last >= 16) {
            last = timer_ticks;

            asm volatile("cli");

            if (mouse_x != last_mx || mouse_y != last_my) {
                add_dirty(last_mx - 2, last_my - 2, CURSOR_W + 4, CURSOR_H + 4);
                add_dirty(mouse_x  - 2, mouse_y  - 2, CURSOR_W + 4, CURSOR_H + 4);
                last_mx = mouse_x;
                last_my = mouse_y;
            }

            if (have_dirty) {
                int W = gfx_get_width(), H = gfx_get_height();
                if (dx0 < 0) dx0 = 0;
                if (dy0 < 0) dy0 = 0;
                if (dx1 > W) dx1 = W;
                if (dy1 > H) dy1 = H;
                int rx = dx0, ry = dy0, rw = dx1 - dx0, rh = dy1 - dy0;
                have_dirty = false;

                // redraw only dirty rectangle 
                gfx_set_clip(rx, ry, rw, rh);
                gfx_fill_rect(rx, ry, rw, rh, 0xFF16091F);
                wm_draw();
                gfx_draw_cursor();
                gfx_reset_clip();

                gfx_present_rect(rx, ry, rw, rh);
            }

            asm volatile("sti");
        }
        asm volatile("hlt");
    }
}