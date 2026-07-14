// header file for the idt
#pragma once
#include <stdint.h>

#define PS2_DATA_PORT 0x60
#define PS2_STATUS_PORT 0x64
#define PS2_CMD_PORT 0x64

struct idt_entry_struct {
    uint16_t isr_low;
    uint16_t selector;
    uint8_t  ist;
    uint8_t  flags;
    uint16_t isr_mid;
    uint32_t isr_high;
    uint32_t zero;
} __attribute__((packed));

struct idt_ptr_struct { // pointer to the idt
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

void initIdt();
void init_mouse();
extern "C" void setIdtGate(uint8_t n, uint64_t handler, uint16_t sel, uint8_t flags);