// header file for io functions
#include "printf.h"

static inline void outb(unsigned short port, unsigned char val) { // write a byte to the port
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline unsigned char inb(unsigned short port) { // read a byte from the port
    unsigned char ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

inline void outw(uint16_t port, uint16_t value) {
    __asm__ __volatile__(
        "outw %0, %1"
        :
        : "a"(value), "Nd"(port)
    );
}

static inline void io_wait(uint16_t port) {
    uint16_t status_port = (port & 0xFF00) ? port + 7 : 0x1F7;
    
    // poll status until busy is clear
    int timeout = 0;
    while (inb(status_port) & 0x80) {
        timeout++;
        if (timeout > 1000000) {
            printf("ATA Timeout: Controller stuck!\n");
            break;
        }
    }
}