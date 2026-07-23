#include <stdint.h>

struct Window {
    int x;
    int y;
    int width;
    int height;
    char title[32]; 
    uint32_t* buffer;
    int layer;
    bool is_dirty;
    bool in_use;
};

extern Window* console_window;

void wm_draw();
void wm_mark_dirty();
void wm_task();
Window* wm_create_window(const char* title, int x, int y, int w, int h);