#pragma once // idk why i need it as once here but i kept getting errors about it just trust the process
#include <stdint.h>

void memset(void *dest, char val, uint32_t count);

void vga_print(const char *str);
