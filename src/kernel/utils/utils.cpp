#include "utils.h"
#include "io.h"
#include "printf.h"
#include <stdint.h>

#define PIC1		0x20		// master adress PIC
#define PIC2		0xA0		// slave adress PIC
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

static int row = 0, col = 0;

// #define MAX_ROWS 2000 // max history rows
// #define SCREEN_ROWS 25
// #define SCREEN_COLS 80

// // ---------------- SAVE CONSOLE HISTORY -----------------
// uint16_t terminal_buffer[MAX_ROWS * SCREEN_COLS];
// int current_total_rows = 0; // how many rows are filled?
// int scroll_offset = 0;      // how many rows up are we from bottom?

// ---------------- VGA FUNCTIONS ----------------

void vga_putc(char c, int txt_col, int bg_col) {
    unsigned char attribute = (bg_col << 4) | (txt_col & 0x0F);

    unsigned short final_pair = (unsigned short)c | (unsigned short)attribute << 8;

    if (c == '\n') {
        col = 0;
        row++;
    } else {
        unsigned short *vidmem = (unsigned short *)0xB8000;
        vidmem[row * 80 + col] = final_pair;
        col++;
    }

    if (row >= 25) {
        row = 0;
        col = 0;
    }
    if (col >= 80) {
        col = 0;
        row++;
    }
}

void vga_print(const char *str, int txt_col, int bg_col) { // adapted to use the same printing method for everything
    for (int i = 0; str[i] != '\0'; i++) {
        vga_putc(str[i], txt_col, bg_col);
    }
}

void hex_dump(void* addr, int len) {
    unsigned char* p = (unsigned char*)addr;
    for (int i = 0; i < len; i += 16) {
        // print offset (current location in buffer)
        print_hex_8bit(i); 
        vga_print(": ", 0xFF, 0x00);

        // print hex values (16 per line)
        for (int j = 0; j < 16; j++) {
            if (i + j < len) {
                print_hex_8bit(p[i + j]);
                vga_print(" ", 0xFF, 0x00);
            } else {
                vga_print("   ", 0xFF, 0x00); // padding
            }
        }

        vga_print(" | ", 0xFF, 0x00);

        // print ascii characters
        for (int j = 0; j < 16; j++) {
            if (i + j < len) {
                unsigned char c = p[i + j];
                // only print printable characters, otherwise print a dot
                if (c >= 32 && c <= 126) {
                    char str[2] = {(char)c, '\0'};
                    vga_print(str, 0xFF, 0x00);
                } else {
                    vga_print(".", 0xFF, 0x00);
                }
            }
        }
        vga_print("\n", 0xFF, 0x00);
    }
}

void print_hex_8bit(uint8_t n) { // converts numbers to hexadecimal
    const char hex_chars[] = "0123456789ABCDEF";
    char hi = hex_chars[(n >> 4) & 0x0F];
    char lo = hex_chars[n & 0x0F];
    
    char str[3] = {hi, lo, '\0'};
    vga_print(str, 0xFF, 0x00);
}

// ---------------- MEMORY OPERATIONS ----------------

extern "C" void memset(void *dest, char val, uint32_t count) {
    char *temp = (char*) dest;
    for (; count != 0; count --) {
        *temp++ = val;
    }
}

static inline uint16_t inw(uint16_t port) { // read a word from a port
    uint16_t ret;
    asm volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void read_disk(uint32_t LBA, uint8_t count, uint32_t target_address) {
    uint8_t status = inb(0x1F7);
    if (status == 0xFF) {
        printf("No drive on port!");
        return;
    }

    inb(0x1F7); // clear any possible remaining errors
    
    // 0xE0 - master drive lba mode
    outb(0x1F6, (0xE0 | ((LBA >> 24) & 0x0F)));
    
    // sector count and lba
    outb(0x1F2, count);
    outb(0x1F3, (uint8_t)LBA);         // lower 8 bits
    outb(0x1F4, (uint8_t)(LBA >> 8));  // middle 8 bits
    outb(0x1F5, (uint8_t)(LBA >> 16)); // high 8 bits
    
    // 0x20 is the read command
    outb(0x1F7, 0x20);

    uint16_t* buffer = (uint16_t*)target_address;

    // loop through sectors and get their data
    for (int s = 0; s < count; s++) {
        // wait for BSY bit 7 to be 0, and DRQ bit 3 to be 1)
        while (!(inb(0x1F7) & 0x08)); 

        // send 256 words (512 bytes) per sector
        for (int i = 0; i < 256; i++) {
            buffer[i] = inw(0x1F0);
        }
        buffer += 256; // move pointer to next sector
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

uint32_t string_to_int(char* str) {
    uint32_t ret = 0;
    for (int i = 0; str[i] != '\0'; ++i) {
        if (str[i] >= '0' && str[i] <= '9') {
            ret = ret * 10 + str[i] - '0';
        }
    }
    return ret;
}

// ----------------- SHELL FUNCTIONS/COMMANDS -----------------
void execute_command(const char* command) {
    if (strcmp(command, "help") == true) {
        vga_print("Available commands:\n", 0xFF, 0x00);
        vga_print("help - Show this help message\n", 0xFF, 0x00);
        vga_print("clear - Clear the screen\n", 0xFF, 0x00);
        vga_print("echo - Display a line of text\n", 0xFF, 0x00);
        vga_print("readdisk - Reads user specified LBA and returns a hex dump of the first 128 bytes", 0xFF, 0x00);
    } 
    
    else if (strcmp(command, "clear") == true) {
        for (int i = 0; i < 80 * 25; i++) {
            ((unsigned short*)0xB8000)[i] = ' ' | (0x07 << 8); // clear screen
        }
        row = 0;
        col = 0; 
    }
    else if (starts_with(command, "echo") == true) {
        vga_print("\n", 0xFF, 0x00);
        vga_print(command + 5, 0xFF, 0x00);
    }
    else if (starts_with(command, "readdisk") == true) {
        const char* arg = command + 9;
        uint32_t lba = string_to_int((char*)arg);
        uint8_t* file_buffer = (uint8_t*)0x20000;

        read_disk(lba, 1, (uint32_t)file_buffer);

        vga_print("\nHex Dump of LBA ", 0x03, 0x00);
        vga_print(arg, 0x03, 0x00);
        vga_print(":\n", 0x03, 0x00);
        
        // first 128 bytes
        hex_dump(file_buffer, 128);
    }
    
    else {
        vga_print("Unknown command: ", 0xFF, 0x00);
        vga_print(command, 0xFF, 0x00);
        vga_print("\n", 0xFF, 0x00);
        vga_print("Type 'help' for a list of commands.", 0xFF, 0x00);
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
    vga_print("A critical error has occured, system has been halted.\n", 0xFF, 0x04);
    while (1) {} // halt the system
}