#include "printf.h"
#include "console.h"
#include "io.h"
#include "utils.h"
#include "gdt.h"
#include "idt.h"
#include "timer.h"
#include "memory.h"

#define SERIAL_PORT 0x3F8

// ---------------- SERIAL INITIALIZATION ----------------
void init_serial() {
    outb(SERIAL_PORT + 1, 0x00);
    outb(SERIAL_PORT + 3, 0x80);
    outb(SERIAL_PORT + 0, 0x03);
    outb(SERIAL_PORT + 1, 0x00);
    outb(SERIAL_PORT + 3, 0x03);
    outb(SERIAL_PORT + 2, 0xC7);
    outb(SERIAL_PORT + 4, 0x0B);
}

void _putchar(char c) {
    while ((inb(SERIAL_PORT + 5) & 0x20) == 0);
    outb(SERIAL_PORT, c);
}

extern "C" int kernel_entry(BootInfo* boot_info) {
    if (boot_info && boot_info->fb.BaseAddress != 0) {
        console_init((uint8_t*)boot_info->fb.BaseAddress, 
                     boot_info->fb.Width, 
                     boot_info->fb.Height, 
                     boot_info->fb.Pitch);
    }

    init_serial();
    initGdt();
    initIdt();
    init_mouse();
    remap_pic();
    init_timer(1000);
    
    init_pmm(boot_info->mem_map_size, boot_info->mem_desc_size);
    init_paging();

    asm volatile ("sti");

    vga_print("========================================\n"
              "==            Solstice OS             ==\n"
              "========================================\n\n$ ", 0xFF, 0x00);

    // PMM test
    // void* test_page = pmm_alloc();
    // printf("Allocated page at: %p\n", test_page);
    
    while (1) { asm volatile ("hlt"); }
    return 1;
}