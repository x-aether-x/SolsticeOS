#include "utils.h"
#include "io.h"
#include <stdint.h>


#define PIC1		0x20		// master adress PIC
#define PIC2		0xA0		// slave adress PIC
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)


extern "C" void memset(void *dest, char val, uint32_t count) {
    char *temp = (char*) dest;
    for (; count != 0; count --) {
        *temp++ = val;
    }
}

void vga_putc(char c, int row, int col) {
    unsigned short *vidmem = (unsigned short *)0xB8000;
    vidmem[row * 80 + col] = (c & 0xFF) | (0x07 << 8);
}

void vga_print(const char *str) {
    volatile unsigned short *vidmem = (unsigned short *)0xB8000; // VGA text buffer
    int row = 0, col = 0;

    while (*str) {
        if (*str == '\n') {
            row++; // Move down a row
            col = 0; // Reset column position
            vidmem = (volatile unsigned short *)0xB8000 + row * 80; // Move to start of next row
        } else {
            vidmem[col++] = (*str & 0xFF) | (0x07 << 8); // White text on black
        }
        str++;
    }
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

// make a memcpy function
// make a memcmp function
// and a memmove function
// gcc and clang make calls to these funcions, so if u dont have them u get cooked