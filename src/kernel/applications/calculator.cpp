#include "calculator.h"
#include "wm.h"
#include "console.h"
#include "cursor.h"
#include "timer.h"
#include <stdint.h>

#define CALC_TITLEBAR_H 24

#define CALC_WIN_W 176
#define CALC_WIN_H 240
#define DISPLAY_H  44
#define GRID_MARGIN 8
#define GRID_GAP    6
#define BTN_W ((CALC_WIN_W - 2 * GRID_MARGIN - 3 * GRID_GAP) / 4)
#define BTN_H 38

Window* calc_window = nullptr;

static const char grid_labels[4][4] = {
    {'1', '2', '3', '/'},
    {'4', '5', '6', '*'},
    {'7', '8', '9', '-'},
    {'C', '0', '=', '+'},
};

static int32_t display_value = 0;
static int32_t pending_value = 0;
static char pending_op = 0;
static bool start_new_number = true;

// mirrors whats been typed until = is pressed, then it shows the output
static char expr_buf[32] = "0";
static int expr_len = 1;
static bool just_evaluated = false;

static void expr_reset() { expr_len = 0; expr_buf[0] = '\0'; }
static void expr_append(char c) {
    if (expr_len < (int)sizeof(expr_buf) - 1) {
        expr_buf[expr_len++] = c;
        expr_buf[expr_len] = '\0';
    }
}

static int grid_x(int col) { return GRID_MARGIN + col * (BTN_W + GRID_GAP); }
static int grid_y(int row) { return DISPLAY_H + GRID_MARGIN + row * (BTN_H + GRID_GAP); }

// fills a rect directly in a window's pixel buffer
static void buf_fill_rect(Window* win, int x, int y, int w, int h, uint32_t color) {
    int x0 = x < 0 ? 0 : x;
    int y0 = y < 0 ? 0 : y;
    int x1 = x + w > win->width ? win->width : x + w;
    int y1 = y + h > win->height ? win->height : y + h;
    for (int row = y0; row < y1; row++) {
        uint32_t* line = win->buffer + (uint64_t)row * win->width;
        for (int col = x0; col < x1; col++) line[col] = color;
    }
}

static void buf_draw_string(Window* win, int x, int y, const char* str, uint32_t fg, uint32_t bg) {
    int pitch_bytes = win->width * 4;
    int cx = x;
    for (; *str; str++) {
        cx += console_draw_glyph(win->buffer, pitch_bytes, cx, y, *str, fg, bg);
    }
}

// negaetives avoiding overflow
static int itoa_dec(int32_t val, char* out) {
    bool neg = val < 0;
    uint32_t uval = neg ? (uint32_t)(-(int64_t)val) : (uint32_t)val;

    char tmp[12];
    int n = 0;
    if (uval == 0) tmp[n++] = '0';
    while (uval > 0) { tmp[n++] = '0' + (uval % 10); uval /= 10; }

    int i = 0;
    if (neg) out[i++] = '-';
    while (n > 0) out[i++] = tmp[--n];
    out[i] = '\0';
    return i;
}

static int32_t apply_op(int32_t a, int32_t b, char op) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return b == 0 ? a : a / b;
        default:  return b;
    }
}

static void draw_all(Window* win) {
    buf_fill_rect(win, 0, 0, win->width, win->height, 0xFF16091F);

    buf_fill_rect(win, 6, 6, win->width - 12, DISPLAY_H - 12, 0xFF0D0617);
    buf_draw_string(win, 12, 16, expr_buf, 0xFFF5EFFF, 0xFF0D0617);

    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            char label = grid_labels[row][col];
            int bx = grid_x(col), by = grid_y(row);
            uint32_t bg = (label >= '0' && label <= '9') ? 0xFF2A1B4A : 0xFF4C2A85;
            buf_fill_rect(win, bx, by, BTN_W, BTN_H, bg);
            char lbl[2] = { label, '\0' };
            buf_draw_string(win, bx + BTN_W / 2 - 4, by + BTN_H / 2 - 8, lbl, 0xFFF5EFFF, bg);
        }
    }
}

static void on_button(char label) {
    if (label >= '0' && label <= '9') {
        if (just_evaluated) { expr_reset(); just_evaluated = false; }
        int d = label - '0';
        display_value = start_new_number ? d : display_value * 10 + d;
        start_new_number = false;
        expr_append(label);
    } 
    else if (label == 'C') {
        display_value = 0;
        pending_value = 0;
        pending_op = 0;
        start_new_number = true;
        just_evaluated = false;
        expr_reset();
        expr_append('0');
    } else if (label == '=') {
        if (pending_op) {
            display_value = apply_op(pending_value, display_value, pending_op);
            pending_op = 0;
        }
        start_new_number = true;
        expr_len = itoa_dec(display_value, expr_buf);
        just_evaluated = true;
    } else { // + - * /
        just_evaluated = false;

        if (pending_op && start_new_number) {
            // swap operator if doubled 
            if (expr_len > 0) expr_buf[expr_len - 1] = label;
        } else {
            if (pending_op) display_value = apply_op(pending_value, display_value, pending_op);
            expr_append(label);
        }

        pending_value = display_value;
        pending_op = label;
        start_new_number = true;
    }
}

static void mark_calc_dirty(Window* win) {
    wm_mark_dirty_rect(win->x, win->y - CALC_TITLEBAR_H, win->width, win->height + CALC_TITLEBAR_H);
}

void calculator_task() {
    Window* win = calc_window;
    uint64_t last = 0;
    uint8_t last_buttons = mouse_buttons;

    asm volatile("cli");
    draw_all(win);
    mark_calc_dirty(win);
    asm volatile("sti");

    for (;;) {
        if (timer_ticks - last >= 16) {
            last = timer_ticks;

            asm volatile("cli");

            bool left_down = mouse_buttons & 0x1;
            bool left_was_down = last_buttons & 0x1;
            last_buttons = mouse_buttons;

            if (left_down && !left_was_down) {
                int lx = mouse_x - win->x;
                int ly = mouse_y - win->y;

                if (lx >= 0 && lx < win->width && ly >= 0 && ly < win->height) {
                    for (int row = 0; row < 4; row++) {
                        for (int col = 0; col < 4; col++) {
                            int bx = grid_x(col), by = grid_y(row);
                            if (lx >= bx && lx < bx + BTN_W && ly >= by && ly < by + BTN_H) {
                                on_button(grid_labels[row][col]);
                                draw_all(win);
                                mark_calc_dirty(win);
                            }
                        }
                    }
                }
            }

            asm volatile("sti");
        }
        asm volatile("hlt");
    }
}
