#include <stdint.h>
#include "io.h"

#define PIT_CMD_PORT 0x43
#define PIT_CH0_PORT 0x40

volatile uint64_t timer_ticks = 0; 

void init_timer(uint32_t frequency) {
    uint32_t divisor = 1193180 / frequency;

    // Command word: Channel 0, lobyte/hibyte, Mode 3 (Square Wave), Binary
    outb(PIT_CMD_PORT, 0x36);

    outb(PIT_CH0_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CH0_PORT, (uint8_t)((divisor >> 8) & 0xFF));
}

void sleep(uint32_t milliseconds) {
    uint64_t target = timer_ticks + milliseconds;
    
    // save pic masks
    uint8_t master_mask = inb(0x21);
    uint8_t slave_mask = inb(0xA1);
    
    // mask all irqs except timer
    outb(0x21, 0xFE); 
    // mask all of slave pic
    outb(0xA1, 0xFF); 

    asm volatile("sti");
    
    while (timer_ticks < target) {
        asm volatile("hlt"); // cpu ignores all interrupts except timer
    }
    
    outb(0x21, master_mask);
    outb(0xA1, slave_mask);
}