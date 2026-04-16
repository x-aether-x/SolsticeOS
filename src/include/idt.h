// header file for the idt
#pragma once
#include <stdint.h>

struct idt_entry_struct {
    uint16_t isr_low;      // lower 16 bits of handler
    uint16_t selector;      // Kernel segment selector (usually 0x08)
    uint8_t  zero;          // must always be zero
    uint8_t  flags;         // types and attributes
    uint16_t isr_high;     // upper 16 bits of handler
} __attribute__((packed));

struct idt_ptr_struct { // pointer to the idt
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct registers {
    uint32_t ds;                                     // pushed manually
    uint32_t eax, ecx, edx, ebx, esp, ebp, esi, edi;  // pushed by pusha
    uint32_t int_no, err_code;                       // int number and err code
    uint32_t eip, cs, eflags, useresp, ss;           // pushed by cpu
}__attribute__((packed));

void initIdt();
extern "C" void setIdtGate(uint8_t n, uint32_t handler, uint16_t sel, uint8_t flags);