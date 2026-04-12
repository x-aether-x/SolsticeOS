#include "utils.h"
#include <stdint.h>

void memset(void *dest, char val, uint32_t count) {
  char *temp = (char*) dest;
  for (; count != 0; count --) {
    *temp++ = val;
  }
}

void vga_print(const char *str) {
    volatile unsigned short *vidmem = (unsigned short *)0xB8000; // VGA text buffer
    int row = 0, col = 0;

    while (*str) {
        if (*str == '\n') {
            row++; // Move down a row
            col = 0; // Reset column position
            vidmem = (volatile unsigned short *)0xB8000 + row * 80; // Move to start of next row
        } else {
            vidmem[col++] = (*str & 0xFF) | (0x07 << 8); // White text on black
        }
        str++;
    }
}
