// idt file used to setup the interrupt descriptor table to allow interrupts in my code

#include <stdint.h>
#include "idt.h"
#include "utils.h"
#include "printf.h"


extern "C" void interrupt_handler(uint8_t interrupt_number) {
    printf("Received Interrupt: %d\n", interrupt_number);
}

idt_entry_struct idt_entries[256];
idt_ptr_struct idt_ptr;

void initIdt() {
    idt_ptr.limit = (sizeof(idt_entry_struct) * 256) - 1;
    idt_ptr.base = (uint32_t)&idt_entries;


}