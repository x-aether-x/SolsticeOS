#include "printf.h"
#include "console.h"
#include "io.h"
#include "utils.h"
#include "gdt.h"
#include "idt.h"

#define SERIAL_PORT 0x3F8

struct FramebufferInfo {
    uint64_t BaseAddress;
    uint32_t Width;
    uint32_t Height;
    uint32_t Pitch;
};

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

    asm volatile ("sti");

    vga_print("========================================\n"
              "==            Solstice OS             ==\n"
              "========================================\n\n$ ", 0xFF, 0x00);

    while (1) { asm volatile ("hlt"); }
    return 1;
}