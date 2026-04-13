// idt file used to setup the interrupt descriptor table to allow interrupts in my code

#include <stdint.h>
#include "idt.h"
#include "utils.h"
#include "printf.h"
#include "io.h"

extern "C" {
    idt_entry_struct idt_entries[256];
    idt_ptr_struct idt_ptr;

    void idt_flush(uint32_t);

    extern void isr0();
    extern void isr1();
    extern void isr2();
    extern void isr3();
    extern void isr4();
    extern void isr5();
    extern void isr6();
    extern void isr7();
    extern void isr8();
    extern void isr9();
    extern void isr10();
    extern void isr11();
    extern void isr12();
    extern void isr13();
    extern void isr14();
    extern void isr15();
    extern void isr16();
    extern void isr17();
    extern void isr18();
    extern void isr19();
    extern void isr20();
    extern void isr21();
    extern void isr22();
    extern void isr23();
    extern void isr24();
    extern void isr25();
    extern void isr26();
    extern void isr27();
    extern void isr28();
    extern void isr29();
    extern void isr30();
    extern void isr31();
}

void* isr_stub_table[] = {
    (void*)isr0, (void*)isr1, (void*)isr2, (void*)isr3, (void*)isr4, (void*)isr5, (void*)isr6, (void*)isr7,
    (void*)isr8, (void*)isr9, (void*)isr10, (void*)isr11, (void*)isr12, (void*)isr13, (void*)isr14, (void*)isr15, (void*)isr16, (void*)isr17, (void*)isr18, (void*)isr19, (void*)isr20, (void*)isr21, (void*)isr22, (void*)isr23,
    (void*)isr24, (void*)isr25, (void*)isr26, (void*)isr27, (void*)isr28, (void*)isr29, (void*)isr30, (void*)isr31
};

extern "C" void setIdtGate(uint8_t vector, uint32_t handler, uint16_t sel, uint8_t flags) {
    idt_entry_struct* descriptor = &idt_entries[vector];

    descriptor->isr_low = (uint32_t)handler & 0xFFFF;
    descriptor->selector = sel; // kernel code segment 
    descriptor->flags = flags; 
    descriptor->isr_high = ((uint32_t)handler >> 16) & 0xFFFF;
    descriptor->zero = 0;
}
 

void initIdt() {
    idt_ptr.limit = (sizeof(idt_entry_struct) * 256) - 1;
    idt_ptr.base = (uint32_t)idt_entries;

    for (int i = 0; i < 32; i++) {
        setIdtGate(i, (uint32_t)isr_stub_table[i], 0x08, 0x8E);
    }

    idt_flush((uint32_t)&idt_ptr);
}

extern "C" void interrupt_handler(uint8_t interrupt_number) {
    printf("Received Interrupt: %d\n", interrupt_number);

    if (interrupt_number >= 0x20) {
        // If it's from the pic, send eoi 
        if (interrupt_number >= 0x28) {
            outb(0xA0, 0x20);
        }
        outb(0x20, 0x20); // send eoi to master
    }
}