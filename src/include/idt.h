// header file for the idt
#pragma once;
#include <stdint.h>

struct idt_entry_struct {
    uint16_t base_low;      // lower 16 bits of handler
    uint16_t selector;      // Kernel segment selector (usually 0x08)
    uint8_t  zero;          // must always be zero
    uint8_t  flags;         // types and attributes
    uint16_t base_high;     // upper 16 bits of handler
} __attribute__((packed));

struct idt_ptr_struct { // pointer to the idt
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

void initIdt();
