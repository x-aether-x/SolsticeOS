#pragma once // helps to remove naming clashes 
#include <stdint.h>
#include <stddef.h>

extern "C" void memset(void *dest, char val, uint64_t count); // set block of memory to a value
extern "C" void memcpy(void *dest, const void *src, uint64_t count);
extern "C" int memcmp(const void* s1, const void* s2, size_t n);
extern "C" void memmove(void *dest, const void *src, uint32_t count);
extern "C" void read_disk(uint32_t LBA, uint8_t count, uint64_t target_address, uint16_t port); // reads disk via ATA_PIO
extern "C" void write_disk(uint32_t LBA, uint8_t count, uint64_t source_address, uint16_t port);

void vga_print(const char *str, int txt_col, int bg_col); // set vga_print 
void vga_putc(char c, int txt_col, int bg_col); // set vga_putc to print a string
void print_hex_8bit(uint8_t n); // bytes to string
void hex_dump(void* addr, int len); // prints hexes instead of weird symbols
void update_hardware_cursor(int col, int row);
const char* next_arg(const char* str); 
void execute_command(const char* command); // execute a command from shell_buffer
void print_prompt(); // colored path + $ // execute a command from shell_buffer

int strlen(const char* str);
bool strcmp(const char *s1, const char *s2); // custom: returns true when equal
char* strcpy(char* dest, const char* src);
char* strcat(char* dest, const char* src);
char *strdup (const char *s);

void remap_pic(); // remap the PIC interrupt vectors

void kernel_panic(); // kernel_panic function that halts the system when an interrupt occurs