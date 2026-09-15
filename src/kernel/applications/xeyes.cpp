#include "xeyes.h"
#include "wm.h"
#include "cursor.h"
#include "timer.h"
#include "task.h"
#include "utils.h"

#define XEYES_WIN_W 160
#define XEYES_WIN_H 100

Window* xeyes_window = nullptr;

static void buf_fill_circle(Window* win, int cx, int cy, int r, uint32_t color) {
    int x0 = cx - r < 0 ? 0 : cx - r;
    int y0 = cy - r < 0 ? 0 : cy - r;
    int x1 = cx + r + 1 > win->width ? win->width : cx + r + 1;
    int y1 = cy + r + 1 > win->height ? win->height : cy + r + 1;

    int r2 = r * r;
    for (int y = y0; y < y1; y++) {
        uint32_t* line = win->buffer + (uint64_t)y * win->width;
        int dy = y - cy;
        for (int x = x0; x < x1; x++) {
            int dx = x - cx;
            if (dx * dx + dy * dy <= r2) line[x] = color;
        }
    }
}

void xeyes_open() {
    if (xeyes_window != nullptr) return;
    xeyes_window = wm_create_window("XEyes", 300, 200, XEYES_WIN_W, XEYES_WIN_H);
    task_create(xeyes_task);
}

void xeyes_task() {
    Window* win = xeyes_window;
    
    uint32_t color_bg    = 0xFFFFCCCC;
    uint32_t color_white = 0xFFFFFFFF;
    uint32_t color_black = 0x00000000;

    int eye_r = 22;      // eyeball radius
    int pupil_r = 7;     // pupil radius
    int max_move = eye_r - pupil_r - 2; 

    int left_cx = win->width / 4;        // x = 40
    int right_cx = (win->width * 3) / 4; // x = 120
    int eye_cy = win->height / 2;        // y = 50

    for (;;) {
        int mouse_win_x = mouse_x - win->x;
        int mouse_win_y = mouse_y - win->y;

        uint32_t total_pixels = (uint32_t)win->width * win->height;
        for (uint32_t i = 0; i < total_pixels; i++) {
            win->buffer[i] = color_bg;
        }

        // outer white
        buf_fill_circle(win, left_cx, eye_cy, eye_r, color_white);
        buf_fill_circle(win, right_cx, eye_cy, eye_r, color_white);

        //left pupil 
        int left_dx = mouse_win_x - left_cx;
        int left_dy = mouse_win_y - eye_cy;
        int left_dist = int_sqrt(left_dx * left_dx + left_dy * left_dy);
        
        int left_pupil_x = left_cx;
        int left_pupil_y = eye_cy;

        if (left_dist > 0) {
            if (left_dist > max_move) {
                left_pupil_x += (left_dx * max_move) / left_dist;
                left_pupil_y += (left_dy * max_move) / left_dist;
            } else {
                left_pupil_x += left_dx;
                left_pupil_y += left_dy;
            }
        }

        // right pupil 
        int right_dx = mouse_win_x - right_cx;
        int right_dy = mouse_win_y - eye_cy;
        int right_dist = int_sqrt(right_dx * right_dx + right_dy * right_dy);

        int right_pupil_x = right_cx;
        int right_pupil_y = eye_cy;

        if (right_dist > 0) {
            if (right_dist > max_move) {
                right_pupil_x += (right_dx * max_move) / right_dist;
                right_pupil_y += (right_dy * max_move) / right_dist;
            } else {
                right_pupil_x += right_dx;
                right_pupil_y += right_dy;
            }
        }

        buf_fill_circle(win, left_pupil_x, left_pupil_y, pupil_r, color_black);
        buf_fill_circle(win, right_pupil_x, right_pupil_y, pupil_r, color_black);

        wm_mark_dirty_rect(win->x, win->y, win->width, win->height);
 
        asm volatile("hlt");
    }
}