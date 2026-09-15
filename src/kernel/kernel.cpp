#include "printf.h"
#include "console.h"
#include "io.h"
#include "utils.h"
#include "gdt.h"
#include "idt.h"
#include "memory.h"
#include "timer.h"
#include "gfx.h"
#include "wm.h"
#include "task.h"

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

extern "C" int kernel_entry(FramebufferInfo* fb_info_ptr) {
    init_serial();
    klog(LOG_INFO, "SolsticeOS booting...");

    FramebufferInfo* fb_info = (FramebufferInfo*)fb_info_ptr;
    if (fb_info && fb_info->BaseAddress != 0) {
        console_init((uint8_t*)fb_info->BaseAddress, fb_info->Width, fb_info->Height, fb_info->Pitch);
        klog(LOG_OK, "Framebuffer console initialized (%ux%u)", fb_info->Width, fb_info->Height);
    } else {
        klog(LOG_WARN, "No framebuffer provided by bootloader");
    }

    initGdt();
    klog(LOG_OK, "GDT initialized");
    initIdt();
    klog(LOG_OK, "IDT initialized");
    remap_pic();
    klog(LOG_OK, "PIC remapped");

    if (fb_info) {
        init_pmm((uint32_t)fb_info->MapSize, (uint32_t)fb_info->DescSize);
        klog(LOG_OK, "Physical memory manager initialized");
    } else {
        klog(LOG_ERROR, "Skipping PMM init: no memory map available");
    }
    init_kmalloc();
    klog(LOG_OK, "Kernel heap (kmalloc) initialized");
    init_paging();
    klog(LOG_OK, "Paging initialized");
    init_tasking();
    klog(LOG_OK, "Tasking initialized");
    gfx_init(fb_info);
    klog(LOG_OK, "Graphics subsystem initialized");
    init_timer(1000);
    klog(LOG_OK, "PIT timer initialized (1000 Hz)");
    init_mouse();
    klog(LOG_OK, "Mouse driver initialized");

    asm volatile ("sti");
    klog(LOG_INFO, "Interrupts enabled, handing off to shell");

    vga_print(".========================================.\n", 0x05, 0x00);
    vga_print("|   *  .    ", 0x06, 0x00);
    vga_print("S o l s t i c e  O S", 0x0C, 0x00);
    vga_print("    .  *  |\n", 0x06, 0x00);
    vga_print("'========================================'\n", 0x05, 0x00);
    vga_print("  mem: ", 0x08, 0x00);
    printf("%u", (unsigned int)(total_free_pages * 4 / 1024));
    vga_print(" MB free\n\n", 0x08, 0x00);
    klog(LOG_OK, "Boot complete, %u MB free", (unsigned int)(total_free_pages * 4 / 1024));
    print_prompt();

    // PMM test
    // void* test_page = pmm_alloc();
    // printf("Allocated page at: %p\n", test_page); 

    while (1) { asm volatile ("hlt"); }
    return 1;
}