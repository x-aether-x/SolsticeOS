#pragma once // helps to remove naming clashes 
#include <stdint.h>

extern "C" void memset(void *dest, char val, uint32_t count); // set block of memory to a value

void vga_print(const char *str); // set vga_print 
void vga_putc(char c, int row, int col); // set vga_putc to print a string

void remap_pic(); // remap the PIC interrupt vectors