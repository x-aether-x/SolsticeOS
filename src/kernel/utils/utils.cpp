#include "utils.h"
#include "io.h"
#include <stdint.h>

#define PIC1		0x20		// master adress PIC
#define PIC2		0xA0		// slave adress PIC
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

static int row = 0, col = 0;

// ---------------- VGA FUNCTIONS ----------------

void vga_putc(char c) {
    unsigned short *vidmem = (unsigned short *)0xB8000;

    if (c == '\n') {
        col = 0;
        row++;
    }
    else {
        vidmem[row * 80 + col] = (c & 0xFF) | (0x07 << 8);
        col++;
    }

    if (col >= 80) {
        row++;
        col = 0;
    }
    if (row >= 25) {
        row = 0;
    }
}

void vga_print(const char *str) { // adapted to use the same printing method for everything
    for (int i = 0; str[i] != '\0'; i++) {
        vga_putc(str[i]);
    }
}

// ---------------- MEMORY OPERATIONS ----------------

extern "C" void memset(void *dest, char val, uint32_t count) {
    char *temp = (char*) dest;
    for (; count != 0; count --) {
        *temp++ = val;
    }
}
// make a memcpy function
// make a memcmp function
// and a memmove function
// gcc and clang make calls to these funcions, so if u dont have them u get cooked


// ----------------- STRING FUNCTIONS ------------------

bool strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++; s2++;
    }

    if (*(unsigned char *)s1 - *(unsigned char *)s2 == 0) {
        return true;
    }
    else {
        return false;
    }
}

int strlen(const char* str) {
    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

bool starts_with(const char* str, const char* prefix) {
    while (*prefix != '\0') {
        if (*str == '\0' || *str != *prefix) {
            return false;
        }
        str++;
        prefix++;
    }
    return true;
}

// ----------------- SHELL FUNCTIONS/COMMANDS -----------------
void execute_command(const char* command) {
    if (strcmp(command, "help") == true) {
        vga_print("Available commands:\n");
        vga_print("help - Show this help message\n");
        vga_print("clear - Clear the screen\n");
        vga_print("echo - Display a line of text");
    } 
    
    else if (strcmp(command, "clear") == true) {
        for (int i = 0; i < 80 * 25; i++) {
            ((unsigned short*)0xB8000)[i] = ' ' | (0x07 << 8); // clear screen
        }
        row = 0;
        col = 0;
  
    }
    else if (starts_with(command, "echo") == true) {
        vga_print("\n");
        vga_print(command + 5);
    } 
    
    else {
        vga_print("Unknown command: ");
        vga_print(command);
        vga_print("\n");
        vga_print("Type 'help' for a list of commands.");
    }
}

// ----------------- MISC -------------------

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

void kernel_panic() {
    vga_print("A critical error has occured, system has been halted.\n");
    while (1) {} // halt the system
}