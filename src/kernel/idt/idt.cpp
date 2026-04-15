// idt file used to setup the interrupt descriptor table to allow interrupts in my code
// also handles keyboard interrupts and prints them

#include <stdint.h>
#include "idt.h"
#include "utils.h"
#include "printf.h"
#include "io.h"

#define MAX_COMMAND_LEN 256
char command_buffer[MAX_COMMAND_LEN];
int buffer_idx = 0;

static const char scancodes_ascii[] = { // i used gemini to make this array dont judge me
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
    '9', '0', '-', '=', '\b',	/* Backspace */
    '\t',			/* Tab */
    'q', 'w', 'e', 'r',	/* 19 */
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    0,			/* 29   - Control */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
    '\'', '`',   0,		/* Left shift */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
    'm', ',', '.', '/',   0,				/* Right shift */
    '*', 0, ' '	/* Space bar */
};

extern "C" {
    idt_entry_struct idt_entries[256];
    idt_ptr_struct idt_ptr;

    void idt_flush(uint32_t);

    extern void isr0(); // i cbf to use a macro
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
    extern void isr32();
    extern void isr33();
    extern void isr34();
    extern void isr35();
    extern void isr36();
    extern void isr37();
    extern void isr38();
    extern void isr39();
    extern void isr40();
    extern void isr41();
    extern void isr42();
    extern void isr43();
    extern void isr44();
    extern void isr45();
    extern void isr46();
    extern void isr47();
    extern void isr48();
    extern void isr49();
    extern void isr50();
    extern void isr51();
    extern void isr52();
    extern void isr53();
    extern void isr54();
    extern void isr55();
    extern void isr56();
    extern void isr57();
    extern void isr58();
    extern void isr59();
    extern void isr60();
    extern void isr61();
    extern void isr62();
    extern void isr63();
    extern void isr64();
    extern void isr65();
    extern void isr66();
    extern void isr67();
    extern void isr68();
    extern void isr69();
    extern void isr70();
    extern void isr71();
    extern void isr72();
    extern void isr73();
    extern void isr74();
    extern void isr75();
    extern void isr76();
    extern void isr77();
    extern void isr78();
    extern void isr79();
    extern void isr80();
    extern void isr81();
    extern void isr82();
    extern void isr83();
    extern void isr84();
    extern void isr85();
    extern void isr86();
    extern void isr87();
    extern void isr88();
    extern void isr89();
    extern void isr90();
    extern void isr91();
    extern void isr92();
    extern void isr93();
    extern void isr94();
    extern void isr95();
    extern void isr96();
    extern void isr97();
    extern void isr98();
    extern void isr99();
    extern void isr100();
    extern void isr101();
    extern void isr102();
    extern void isr103();
    extern void isr104();
    extern void isr105();
    extern void isr106();
    extern void isr107();
    extern void isr108();
    extern void isr109();
    extern void isr110();
    extern void isr111();
    extern void isr112();
    extern void isr113();
    extern void isr114();
    extern void isr115();
    extern void isr116();
    extern void isr117();
    extern void isr118();
    extern void isr119();
    extern void isr120();
    extern void isr121();
    extern void isr122();
    extern void isr123();
    extern void isr124();
    extern void isr125();
    extern void isr126();
    extern void isr127();
    extern void isr128();
    extern void isr129();
    extern void isr130();
    extern void isr131();
    extern void isr132();
    extern void isr133();
    extern void isr134();
    extern void isr135();
    extern void isr136();
    extern void isr137();
    extern void isr138();
    extern void isr139();
    extern void isr140();
    extern void isr141();
    extern void isr142();
    extern void isr143();
    extern void isr144();
    extern void isr145();
    extern void isr146();
    extern void isr147();
    extern void isr148();
    extern void isr149();
    extern void isr150();
    extern void isr151();
    extern void isr152();
    extern void isr153();
    extern void isr154();
    extern void isr155();
    extern void isr156();
    extern void isr157();
    extern void isr158();
    extern void isr159();
    extern void isr160();
    extern void isr161();
    extern void isr162();
    extern void isr163();
    extern void isr164();
    extern void isr165();
    extern void isr166();
    extern void isr167();
    extern void isr168();
    extern void isr169();
    extern void isr170();
    extern void isr171();
    extern void isr172();
    extern void isr173();
    extern void isr174();
    extern void isr175();
    extern void isr176();
    extern void isr177();
    extern void isr178();
    extern void isr179();
    extern void isr180();
    extern void isr181();
    extern void isr182();
    extern void isr183();
    extern void isr184();
    extern void isr185();
    extern void isr186();
    extern void isr187();
    extern void isr188();
    extern void isr189();
    extern void isr190();
    extern void isr191();
    extern void isr192();
    extern void isr193();
    extern void isr194();
    extern void isr195();
    extern void isr196();
    extern void isr197();
    extern void isr198();
    extern void isr199();
    extern void isr200();
    extern void isr201();
    extern void isr202();
    extern void isr203();
    extern void isr204();
    extern void isr205();
    extern void isr206();
    extern void isr207();
    extern void isr208();
    extern void isr209();
    extern void isr210();
    extern void isr211();
    extern void isr212();
    extern void isr213();
    extern void isr214();
    extern void isr215();
    extern void isr216();
    extern void isr217();
    extern void isr218();
    extern void isr219();
    extern void isr220();
    extern void isr221();
    extern void isr222();
    extern void isr223();
    extern void isr224();
    extern void isr225();
    extern void isr226();
    extern void isr227();
    extern void isr228();
    extern void isr229();
    extern void isr230();
    extern void isr231();
    extern void isr232();
    extern void isr233();
    extern void isr234();
    extern void isr235();
    extern void isr236();
    extern void isr237();
    extern void isr238();
    extern void isr239();
    extern void isr240();
    extern void isr241();
    extern void isr242();
    extern void isr243();
    extern void isr244();
    extern void isr245();
    extern void isr246();
    extern void isr247();
    extern void isr248();
    extern void isr249();
    extern void isr250();
    extern void isr251();
    extern void isr252();
    extern void isr253();
    extern void isr254();
    extern void isr255();
}

void* isr_stub_table[] = {
    (void*)isr0, (void*)isr1, (void*)isr2, (void*)isr3, (void*)isr4, (void*)isr5, (void*)isr6, (void*)isr7,
    (void*)isr8, (void*)isr9, (void*)isr10, (void*)isr11, (void*)isr12, (void*)isr13, (void*)isr14, (void*)isr15, 
    (void*)isr16, (void*)isr17, (void*)isr18, (void*)isr19, (void*)isr20, (void*)isr21, (void*)isr22, (void*)isr23,
    (void*)isr24, (void*)isr25, (void*)isr26, (void*)isr27, (void*)isr28, (void*)isr29, (void*)isr30, (void*)isr31,
    (void*)isr32, (void*)isr33, (void*)isr34, (void*)isr35, (void*)isr36, (void*)isr37, (void*)isr38, (void*)isr39,
    (void*)isr40, (void*)isr41, (void*)isr42, (void*)isr43, (void*)isr44, (void*)isr45, (void*)isr46, (void*)isr47,
    (void*)isr48, (void*)isr49, (void*)isr50, (void*)isr51, (void*)isr52, (void*)isr53, (void*)isr54, (void*)isr55,
    (void*)isr56, (void*)isr57, (void*)isr58, (void*)isr59, (void*)isr60, (void*)isr61, (void*)isr62, (void*)isr63,
    (void*)isr64, (void*)isr65, (void*)isr66, (void*)isr67, (void*)isr68, (void*)isr69, (void*)isr70, (void*)isr71, 
    (void*)isr72, (void*)isr73, (void*)isr74, (void*)isr75, (void*)isr76, (void*)isr77, (void*)isr78, (void*)isr79,
    (void*)isr80, (void*)isr81, (void*) isr82, (void*) isr83, (void*) isr84, (void*) isr85, (void*) isr86, 
    (void*) isr87, (void*) isr88, (void*) isr89, (void*) isr90, (void*) isr91, (void*) isr92, (void*) isr93, (void*) isr94,
    (void*) isr95, (void*) isr96, (void*) isr97, (void*) isr98, (void*) isr99, (void*) isr100, (void*) isr101,
    (void*) isr102, (void*) isr103, (void*) isr104, (void*) isr105, (void*) isr106, (void*) isr107, (void*) isr108, 
    (void*) isr109, (void*) isr110, (void*) isr111, (void*) isr112, (void*) isr113, (void*) isr114, (void*) isr115,
    (void*) isr116, (void*) isr117, (void*) isr118, (void*) isr119, (void*) isr120, (void*) isr121, (void*) isr122,
    (void*) isr123, (void*) isr124, (void*) isr125, (void*) isr126, (void*) isr127, (void*) isr128, (void*) isr129,
    (void*) isr130, (void*) isr131, (void*) isr132, (void*) isr133, (void*) isr134, (void*) isr135, (void*) isr136,
    (void*) isr137, (void*) isr138, (void*) isr139, (void*) isr140, (void*) isr141, (void*) isr142, (void*) isr143,
    (void*) isr144, (void*) isr145, (void*) isr146, (void*) isr147, (void*) isr148, (void*) isr149, (void*) isr150, 
    (void*) isr151, (void*) isr152, (void*) isr153, (void*) isr154, (void*) isr155, (void*) isr156, (void*) isr157,
    (void*) isr158, (void*) isr159, (void*) isr160, (void*) isr161, (void*) isr162, (void*) isr163, (void*) isr164, 
    (void*) isr165, (void*) isr166, (void*) isr167, (void*) isr168, (void*) isr169, (void*) isr170, (void*) isr171, 
    (void*) isr172, (void*) isr173, (void*) isr174, (void*) isr175, (void*) isr176, (void*) isr177, (void*) isr178, 
    (void*) isr179, (void*) isr180, (void*) isr181, (void*) isr182, (void*) isr183, (void*) isr184, (void*) isr185, 
    (void*) isr186, (void*) isr187, (void*) isr188, (void*) isr189, (void*) isr190, (void*) isr191, (void*) isr192, 
    (void*) isr193, (void*) isr194, (void*) isr195, (void*) isr196, (void*) isr197, (void*) isr198, (void*) isr199, 
    (void*) isr200, (void*) isr201, (void*) isr202, (void*) isr203, (void*) isr204, (void*) isr205, (void*) isr206, 
    (void*) isr207, (void*) isr208, (void*) isr209, (void*) isr210, (void*) isr211, (void*) isr212, (void*) isr213, 
    (void*) isr214, (void*) isr215, (void*) isr216, (void*) isr217, (void*) isr218, (void*) isr219, (void*) isr220, 
    (void*) isr221, (void*) isr222, (void*) isr223, (void*) isr224, (void*) isr225, (void*) isr226, (void*) isr227, 
    (void*) isr228, (void*) isr229, (void*) isr230, (void*) isr231, (void*) isr232, (void*) isr233, (void*) isr234, 
    (void*) isr235, (void*) isr236, (void*) isr237, (void*) isr238, (void*) isr239, (void*) isr240, (void*) isr241, 
    (void*) isr242, (void*) isr243, (void*) isr244, (void*) isr245, (void*) isr246, (void*) isr247, (void*) isr248, 
    (void*) isr249, (void*) isr250, (void*) isr251, (void*) isr252, (void*) isr253, (void*) isr254, (void*) isr255
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

     for (int i = 0; i < 256; i++) { // fake value
        setIdtGate(i, (uint32_t)isr0, 0x08, 0x8E);
    }

    for (int i = 0; i < 34; i++) { // write all 256 isr stubs to the idt
        setIdtGate(i, (uint32_t)isr_stub_table[i], 0x08, 0x8E);
    }

    idt_flush((uint32_t)&idt_ptr);
}

extern "C" void interrupt_handler(struct registers* regs) { // printfs kept crashing the kernel becasue of stack overflow so im gonna try avoiding them for now
    if (regs->int_no >= 0x20) {
        // If it's from the pic, send eoi 
        if (regs->int_no >= 0x28) {
            outb(0xA0, 0x20);
        }
        outb(0x20, 0x20); // send eoi to master
    }
 
    if (regs->int_no == 33) {
        uint8_t scancode = inb(0x60);
        
        if (scancode & 0x80) {
            return; 
        }
        
        if (scancode == 0x1C) {
            vga_print("\n", 0x0F, 0x00); // newline on enter
           buffer_idx = 0; // reset buffer index for new command
        } 
        
        else if (scancode < sizeof(scancodes_ascii)) {
            char c = scancodes_ascii[scancode];
            if (c != 0 && c != '\n') {
                // printf("Key Pressed: %c\n", c);
                // printf("Scancode: 0x%02X\n", scancode);
                vga_putc(c, 0x0F, 0x00); // white on black
            }
        }
    }
};


// if interrupt is less than 32 && it happened in userspace:
//     send a SIGKILL
// if interrupt is less than 32 && it happened in kernelspace:
//     panic kernel and print error code with interupt number
//
// currently not sure how to detect userspace vs kernelspace, but this is here for when i learn how