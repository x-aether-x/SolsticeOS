#pragma once // helps to remove naming clashes 
#include <stdint.h>

void memset(void *dest, char val, uint32_t count); // set block of memory to a value

void vga_print(const char *str); // set vga_print 