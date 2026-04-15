#pragma once // helps to remove naming clashes 
#include <stdint.h>

extern "C" void memset(void *dest, char val, uint32_t count); // set block of memory to a value

void vga_print(const char *str, int txt_colour, int bg_colour); // set vga_print 
void vga_putc(char c, int txt_colour, int bg_colour); // set vga_putc to print a character

void remap_pic(); // remap the PIC interrupt vectors
void update_hardware_cursor(int x, int y); // update the hardware cursor position