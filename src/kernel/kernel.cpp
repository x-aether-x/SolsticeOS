#include "printf.h"
#include "io.h"
#include "utils.h"
#include "gdt.h"
#include "idt.h"

#define SERIAL_PORT 0x3F8 // COM1

void init_serial() {
    outb(SERIAL_PORT + 1, 0x00); // Disable interrupts
    outb(SERIAL_PORT + 3, 0x80); // Enable DLAB to set baud rate
    outb(SERIAL_PORT + 0, 0x03); // Set divisor to 3 (38400 baud)
    outb(SERIAL_PORT + 1, 0x00);
    outb(SERIAL_PORT + 3, 0x03); // 8 bits, no parity, one stop bit
    outb(SERIAL_PORT + 2, 0xC7); // Enable FIFO, clear, set 14-byte threshold
    outb(SERIAL_PORT + 4, 0x0B); // IRQs enabled, RTS/DSR set
}

void _putchar(char c) { // helper function for printf
    while ((inb(SERIAL_PORT + 5) & 0x20) == 0);
    outb(SERIAL_PORT, c);
}

int main() {
    init_serial();
    initGdt();
    printf("GDT initialised \n");
    initIdt();
    printf("IDT initialised \n");

    remap_pic(); // remap pic to allow keyboard interrupts
    printf("PIC remapped, enabling IDT... \n");
    
    outb(0x21, 0xFD); // unmask keyboard interrupts

    asm volatile ("sti"); // enable interrupt
    printf("Interrupts enabled \n");

    vga_print("========================================\n"
              "==            Solstice OS             ==\n" // switched to eqals signs cos it looks nicer
              "========================================\n"
              "\n", 0x02, 0x00 // green text on black background
              );

    while (1) {}
    return 1;
}
