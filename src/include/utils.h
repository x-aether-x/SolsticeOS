#pragma once // helps to remove naming clashes 
#include <stdint.h>

extern "C" void memset(void *dest, char val, uint32_t count); // set block of memory to a value
void read_disk(uint32_t LBA, uint8_t count, uint32_t target_address); // reads disk via ATA_PIO

void vga_print(const char *str, int txt_col, int bg_col); // set vga_print 
void vga_putc(char c, int txt_col, int bg_col); // set vga_putc to print a string
void print_hex_8bit(uint8_t n); // bytes to string
void hex_dump(void* addr, int len); // prints hexes instead of weird symbols

void execute_command(const char* command); // execute a command from shell_buffer

void remap_pic(); // remap the PIC interrupt vectors

void kernel_panic(); // kernel_panic function that halts the system when an interrupt occurs