#include <stdint.h>
#include "wm.h"
#include "memory.h"
#include "utils.h"
#include "gfx.h"
#include "timer.h"
#include "cursor.h"

#define TITLEBAR_H 24
#define MAX_WINDOWS 8

Window windows[MAX_WINDOWS];
int window_count = 0;

Window* console_window = nullptr;

static volatile bool wm_dirty = true;
void wm_mark_dirty() { wm_dirty = true; }

Window* wm_create_window(const char* title, int x, int y, int w, int h) {
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (!windows[i].in_use) {
            static uint64_t wm_heap_next = 0x1400000;
            
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
    // build a list of in-use window indices sorted by layer (insertion sort, 8 max)
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

        // title bar sits above the client area
        gfx_fill_rect(win->x, win->y - TITLEBAR_H, win->width, TITLEBAR_H, 0xFF4C2A85);
        gfx_draw_string(win->x + 6, win->y - TITLEBAR_H + 4, win->title, 0xFFF5EFFF, 0xFF4C2A85);

        // close button
        gfx_fill_rect(win->x + win->width - TITLEBAR_H + 4, win->y - TITLEBAR_H + 4,
                      TITLEBAR_H - 8, TITLEBAR_H - 8, 0xFFFF8FA3);

        // client area
        gfx_blit(win->x, win->y, win->buffer, win->width, win->height);

        // border around the whole thing
        gfx_draw_rect(win->x - 1, win->y - TITLEBAR_H - 1,
                      win->width + 2, win->height + TITLEBAR_H + 2, 1, 0xFF9D7BEA);
    }
}

void wm_task() {
    uint64_t last = 0;
    int last_mx = -1, last_my = -1;

    for (;;) {
        if (timer_ticks - last >= 16) {
            last = timer_ticks;

            // only redraw if cursor moved or something is dirty
            bool mouse_moved = (mouse_x != last_mx || mouse_y != last_my);
            if (mouse_moved || wm_dirty) {
                last_mx = mouse_x;
                last_my = mouse_y;
                wm_dirty = false;

                asm volatile("cli");
                gfx_clear(0xFF16091F);
                wm_draw();
                gfx_draw_cursor();
                gfx_present();
                asm volatile("sti");
            }
        }
        asm volatile("hlt");
    }
}