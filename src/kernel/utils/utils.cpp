#include "utils.h"
#include "io.h"
#include "printf.h"
#include <stdint.h>

int cursor_x = 0, cursor_y = 0;

#define PIC1		0x20		// master adress PIC
#define PIC2		0xA0		// slave adress PIC
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

// --------------- VGA FUNCTIONS ---------------

void vga_putc(char c, int txt_colour, int bg_colour) {
    volatile unsigned short *vidmem = (unsigned short *)0xB8000;
    
    // write to absolute position
    vidmem[cursor_y * 80 + cursor_x] = (c & 0xFF) | ((txt_colour & 0x0F) << 8) | ((bg_colour & 0x0F) << 12);
    
    cursor_x++;

    // text wrapping 
    if (cursor_x >= 80) {
        cursor_x = 0;
        cursor_y++;
    }
    if (cursor_y >= 25) { // bad scrolling but whatever (should be writing previous lines to memory)
        cursor_y = 0;
    }
}

void vga_print(const char *str, int txt_colour, int bg_colour) {
    volatile unsigned short *vidmem = (volatile unsigned short *)0xB8000; // VGA text buffer

    while (*str) {
        if (*str == '\n') {
            cursor_y++;
            cursor_x = 0;
        } else {
            vidmem[cursor_y * 80 + cursor_x] = (*str & 0xFF) | ((txt_colour & 0x0F) << 8) | ((bg_colour & 0x0F) << 12);
            cursor_x++;
        }

        if (cursor_x >= 80) {
            cursor_x = 0;
            cursor_y++;
        }
        
        if (cursor_y >= 25) {
        cursor_y = 0;
        }
        str++;
    }
}

void update_hardware_cursor(int x, int y) {
    uint16_t pos = y * 80 + x;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void remap_pic() {
    outb(PIC1_COMMAND, 0x11); // init in cascade mode
    outb(PIC2_COMMAND, 0x11);

    // vector offset
    outb(PIC1_DATA, 0x20); // master
    outb(PIC2_DATA, 0x28); // slave

    outb(PIC1_DATA, 0x04); // inform master slave at IRQ2

    outb(PIC2_DATA, 0x02); // inform slave cascade id (0000 0010)

    // set to 8086/88 (MCS-80/85) mode
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

    // mask interrupts until idt is setup
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}


// --------------- MEMORY FUNCTIONS ---------------
// gcc and clang make calls to these funcions, so if u dont have them u get cooked

extern "C" void memset(void *dest, char val, uint32_t count) {
    char *temp = (char*) dest;
    for (; count != 0; count --) {
        *temp++ = val;
    }
}

extern "C" void memcpy(void *dest, const void *src, uint32_t count) {
    char *temp_dest = (char*) dest;
    const char *temp_src = (const char*) src;
    for (; count != 0; count --) {
        *temp_dest++ = *temp_src++;
    }
}

extern "C" int memcmp(const void *ptr1, const void *ptr2, uint32_t count) {
    const char *temp_ptr1 = (const char*) ptr1;
    const char *temp_ptr2 = (const char*) ptr2;
    for (; count != 0; count --) {
        if (*temp_ptr1 != *temp_ptr2) {
            return *temp_ptr1 - *temp_ptr2;
        }
        temp_ptr1++;
        temp_ptr2++;
    }
    return 0; // equal
}

extern "C" void memmove(void *dest, const void *src, uint32_t count) {
    char *temp_dest = (char*) dest;
    const char *temp_src = (const char*) src;

    if (temp_dest < temp_src) {
        for (; count != 0; count --) {
            *temp_dest++ = *temp_src++;
        }
    } else {
        temp_dest += count - 1;
        temp_src += count - 1;
        for (; count != 0; count --) {
            *temp_dest-- = *temp_src--;
        }
    }
}