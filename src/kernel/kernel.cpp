#include "printf.h"
#include "console.h"
#include "io.h"
#include "utils.h"
#include "gdt.h"
#include "idt.h"
#include "memory.h"
#include "timer.h"
#include "gfx.h"

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

extern "C" int kernel_entry(FramebufferInfo* fb_info_ptr) {
    FramebufferInfo* fb_info = (FramebufferInfo*)fb_info_ptr;
    if (fb_info && fb_info->BaseAddress != 0) {
        console_init((uint8_t*)fb_info->BaseAddress, fb_info->Width, fb_info->Height, fb_info->Pitch);
    }

    init_serial();
    initGdt();
    initIdt();
    remap_pic();
    init_paging();

    if (fb_info) {
        init_pmm((uint32_t)fb_info->MapSize, (uint32_t)fb_info->DescSize);
    }
    init_kmalloc();
    gfx_init(fb_info);
    init_timer(1000);
    init_mouse();

    asm volatile ("sti");

    vga_print(".========================================.\n", 0x05, 0x00);
    vga_print("|   *  .    ", 0x06, 0x00);
    vga_print("S o l s t i c e  O S", 0x0C, 0x00);
    vga_print("    .  *  |\n", 0x06, 0x00);
    vga_print("'========================================'\n", 0x05, 0x00);
    vga_print("  mem: ", 0x08, 0x00);
    printf("%u", (unsigned int)(total_free_pages * 4 / 1024));
    vga_print(" MB free\n\n", 0x08, 0x00);
    print_prompt();

    // PMM test
    // void* test_page = pmm_alloc();
    // printf("Allocated page at: %p\n", test_page); 

    while (1) { asm volatile ("hlt"); }
    return 1;
}