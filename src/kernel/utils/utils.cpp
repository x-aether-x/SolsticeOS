#include "utils.h"
#include "io.h"
#include "printf.h"
#include "console.h"
#include "ext2.h"
#include <stdint.h>
#include <stddef.h>

#define PIC1		0x20		// master adress PIC
#define PIC2		0xA0		// slave adress PIC
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

static int row = 0, col = 0;

#define ROOT_INODE 2

// #define MAX_ROWS 2000 // max history rows
// #define SCREEN_ROWS 25
// #define SCREEN_COLS 80

// // ---------------- SAVE CONSOLE HISTORY -----------------
// uint16_t terminal_buffer[MAX_ROWS * SCREEN_COLS];
// int current_total_rows = 0; // how many rows are filled?
// int scroll_offset = 0;      // how many rows up are we from bottom?

// ------------------ MISC ----------------------
bool is_number(const char* str) {
    if (*str == '\0') return false;
    while (*str != '\0') {
        if (*str < '0' || *str > '9') {
            return false;
        }
        str++;
    }
    return true;
}


// ---------------- MEMORY OPERATIONS ----------------

extern "C" void memset(void *dest, char val, uint64_t count) {
    char *temp = (char*) dest;
    for (; count != 0; count --) {
        *temp++ = val;
    }
}

extern "C" void memcpy(void *dest, const void *src, uint64_t count) {
    char *temp_dest = (char*) dest;
    const char *temp_src = (const char*) src;
    for (; count != 0; count --) {
        *temp_dest++ = *temp_src++;
    }
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

extern "C" int memcmp(const void* s1, const void* s2, size_t n) {
    const unsigned char *p1 = (const unsigned char*)s1;
    const unsigned char *p2 = (const unsigned char*)s2;
    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) return p1[i] - p2[i];
    }
    return 0;
}

static inline uint16_t inw(uint16_t port) { // read a word from a port
    uint16_t ret;
    asm volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

extern "C" void read_disk(uint32_t LBA, uint8_t count, uint64_t target_address, uint16_t port) { // ive had to rewrite ts function like a thousand times oml </3
    uint16_t* buffer = (uint16_t*)target_address;

    outb(port + 6, (0xF0 | ((LBA >> 24) & 0x0F)));

    for (int i = 0; i < 4; i++) {
        inb(port + 7);
    }

    outb(port + 2, count);
    outb(port + 3, (uint8_t)LBA);
    outb(port + 4, (uint8_t)(LBA >> 8));
    outb(port + 5, (uint8_t)(LBA >> 16));
    outb(port + 7, 0x20);

    for (int s = 0; s < count; s++) {
        int timeout = 100000;
        bool has_error = false;
        
        while (timeout > 0) {
            uint8_t status = inb(port + 7);
    
            if ((status & 0x01) || (status & 0x20)) {
                printf("ATA Error on port %x! LBA: %u\n", port, LBA);
                has_error = true;
                break;
            }
            
            if (!(status & 0x80) && (status & 0x08)) {
                break; 
            }
            
            timeout--;
        }
        if (timeout == 0) {
            printf("ATA Timeout! Port %x unresponsive.\n", port);
            return;
        }
        if (has_error) {
            return;
        }
        for (int i = 0; i < 256; i++) {
            *buffer++ = inw(port);
        } 
        for (int i = 0; i < 4; i++) {
            inb(port + 7);
        }
    }
}

extern "C" void write_disk(uint32_t LBA, uint8_t count, uint64_t source_address, uint16_t port) {
    uint16_t* buffer = (uint16_t*)source_address;

    outb(port + 6, (0xF0 | ((LBA >> 24) & 0x0F)));

    // delay
    for (int i = 0; i < 4; i++) {
        inb(port + 7);
    }

    outb(port + 2, count);
    outb(port + 3, (uint8_t)LBA);
    outb(port + 4, (uint8_t)(LBA >> 8));
    outb(port + 5, (uint8_t)(LBA >> 16));
    
    // ATA write
    outb(port + 7, 0x30); 

    for (int s = 0; s < count; s++) {
        int timeout = 100000;
        bool has_error = false;
        
        // wait for drive to be ready
        while (timeout > 0) {
            uint8_t status = inb(port + 7);
    
            if ((status & 0x01) || (status & 0x20)) {
                printf("ATA Write Error on port %x! LBA: %u\n", port, LBA);
                has_error = true;
                break;
            }
            
            if (!(status & 0x80) && (status & 0x08)) {
                break; 
            }
            
            timeout--;
        }
        if (timeout == 0 || has_error) {
            printf("ATA Write failed or timed out!\n");
            return;
        }
        for (int i = 0; i < 256; i++) {
            outw(port, *buffer++);
        }
        for (int i = 0; i < 4; i++) inb(port + 7); // delay (again)
    }

    // flush cache ONCE after ALL sectors, then wait for BSY to clear
    outb(port + 7, 0xE7);
    while (inb(port + 7) & 0x80);
}

// gcc and clang make calls to these funcions, so if u dont have them u get cooked

// ---------------- VGA FUNCTIONS ----------------

void update_hardware_cursor(int col, int row) {
    uint16_t pos = row * 80 + col;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));

    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void scroll_screen() {
    unsigned short *vidmem = (unsigned short *)0xB8000;
    memmove((uint8_t*)vidmem, (uint8_t*)vidmem + (80*2), 24 * 80 * 2);
    for (int i = 0; i < 80; i++) {
        vidmem[80*24 + i] = ' ' | (0x07 << 8); // clear last line 
    }
    row = 24;
    col = 0;
    update_hardware_cursor(col, row);
}

void vga_putc(char c, int txt_col, int bg_col) {
    (void)txt_col;
    (void)bg_col;
    console_putc(c);
}

void vga_print(const char *str, int txt_col, int bg_col) {
    (void)txt_col;
    (void)bg_col;
    console_puts(str);
}

void hex_dump(void* addr, int len) {
    unsigned char* p = (unsigned char*)addr;
    for (int i = 0; i < len; i += 16) {
        // print offset (current location in buffer)
        print_hex_8bit((i >> 8) & 0xFF); // high byte
        print_hex_8bit(i & 0xFF); // low byte
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

char* strcpy(char* dest, const char* src) {
    if (dest == nullptr || src == nullptr) {
        return dest;
    }

    char* ptr = dest;
    while (*src != '\0') {
        *ptr++ = *src++;
    }
    *ptr = '\0';
    return dest;
}

char* strcat(char* dest, const char* src) {
    if (dest == nullptr || src == nullptr) {
        return dest;
    }

    char* ptr = dest;
    while (*ptr != '\0') {
        ptr++;
    }

    while (*src != '\0') {
        *ptr++ = *src++;
    }
    *ptr = '\0';
    return dest;
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

uint32_t string_to_int(const char* str) {
    uint32_t ret = 0;
    int base = 10;
    
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        base = 16;
        str += 2;
    }

    while (*str != '\0' && *str != ' ') {
        if (base == 10 && *str >= '0' && *str <= '9') {
            ret = ret * 10 + (*str - '0');
        } 
        else if (base == 16) {
            if (*str >= '0' && *str <= '9') {
                ret = ret * 16 + (*str - '0');
            } else if (*str >= 'a' && *str <= 'f') {
                ret = ret * 16 + (*str - 'a' + 10);
            } else if (*str >= 'A' && *str <= 'F') {
                ret = ret * 16 + (*str - 'A' + 10);
            } else {
                break;
            }
        } else {
            break; 
        }
        str++;
    }
    return ret;
}

const char* next_arg(const char* str) {
    while (*str == ' ') str++;
    return str;
}

// ----------------- SHELL FUNCTIONS/COMMANDS -----------------
void execute_command(const char* command) {
    if (strcmp(command, "help") == true) {
        vga_print("Available commands:\n", 0xFF, 0x00);
        vga_print("help - Show this help message\n", 0xFF, 0x00);
        vga_print("clear - Clear the screen\n", 0xFF, 0x00);
        vga_print("echo <TEXT> - Display a line of text\n", 0xFF, 0x00);
        vga_print("readdisk <SEGMENT> - Reads user specified LBA and returns a hex dump of the chosen sector\n", 0xFF, 0x00);
        vga_print("ls - Lists the contents of the current directory\n", 0xFF, 0x00);
        vga_print("cd <DIR> - Change directory (cd .. to go up, cd / or bare cd for root)\n", 0xFF, 0x00);
        vga_print("mkdir <DIR_NAME> - Creates a new directory at the current path location", 0xFF, 0x00);
    }
    
    else if (strcmp(command, "clear") == true) {
        console_clear();
    }
    else if (starts_with(command, "echo") == true) {
        vga_print("\n", 0xFF, 0x00);
        vga_print(command + 5, 0xFF, 0x00);
    }
    else if (starts_with(command, "readdisk") == true) {
        const char* arg = command + 9;
        uint32_t lba = string_to_int((char*)arg);
        if (is_number(arg) == true) {
            uint8_t* file_buffer = (uint8_t*)0x20000;

            read_disk(lba, 1, (uintptr_t)file_buffer, 0x1F0);

            vga_print("\nHex Dump of LBA ", 0x03, 0x00);
            vga_print(arg, 0x03, 0x00);
            vga_print(":\n", 0x03, 0x00);
            
            // full sector (512 bytes)
            hex_dump(file_buffer, 512);
        }
        else {
            vga_print("Error: Invalid LBA Specified\n", 0xff, 0x00);
        }
    }
    else if (strcmp(command, "ls") == true) {
        ext2_ls(0x1F0);
    }
    else if (starts_with(command, "cd") == true && (command[2] == ' ' || command[2] == '\0')) {
        const char* dir_name = next_arg(command + 2);
        if (strlen(dir_name) == 0) {
            ext2_cd("/", 0x1F0); // bare cd goes to root
        } else {
            ext2_cd(dir_name, 0x1F0);
        }
    }
    else if (starts_with(command, "mkdir") == true) {
        const char* dir_name = next_arg(command + 5);
        if (strlen(dir_name) == 0) {
            vga_print("mkdir: missing operand\n", 0xFF, 0x00);
        } else {
            vga_print("Creating directory: ", 0xFF, 0x00);
            vga_print(dir_name, 0xFF, 0x00);
            vga_print("\n", 0xFF, 0x00);

            ext2_mkdir(dir_name, g_current_dir, 0x1F0);
        }
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
    outb(PIC1_DATA, 0xFD); // unmask IRQ1 (keyboard) only
    outb(PIC2_DATA, 0xFF);
}

void kernel_panic() {
    vga_print("A critical error has occured, system has been halted.\n", 0xFF, 0x04);
    while (1) {} // halt the system
}