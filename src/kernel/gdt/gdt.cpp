#include <stdint.h>
#include "gdt.h"
#include "printf.h"

extern "C" {
    void gdt_flush(uint64_t); // had name mangling issues, so declared these in c
    void tss_flush();
}

gdt_entry_struct gdt_entries[6]; // array to store entries
gdt_ptr_struct gdt_ptr; // ptr to point to gdt array 

struct tss_entry_struct_64 { // task state segment struct (tss) updated for 64 bit mode
    uint32_t reserved0;
    uint64_t rsp0; // stack pointer for loading when going into kernel mode
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomap_base;
} __attribute__((packed));

struct tss_entry_struct_64 tss_entry; // create instance of tss_entry

void setGdtGate(uint32_t num, uint64_t base, uint32_t limit, uint8_t access, uint8_t gran) { // set gdt gate function
    gdt_entries[num].base_low    = (base & 0xFFFF);
    gdt_entries[num].base_middle = ((base >> 16) & 0xFF);
    gdt_entries[num].access      = access;
    gdt_entries[num].flags       = gran;
    gdt_entries[num].base_high   = ((base >> 24) & 0xFF);
    gdt_entries[num].limit       = (limit & 0xFFFF);
}

void writeTSS(uint32_t num, uint16_t ss0, uint64_t esp0) { // set tss function
    uint64_t base = (uint64_t)&tss_entry;
    uint32_t limit = sizeof(tss_entry) - 1;

    setGdtGate(num, base, limit, 0x89, 0x00); // write lower 8 bytes of tss entry into gdt slot

    uint32_t* high_descriptor = (uint32_t*)&gdt_entries[num + 1]; // in 64 bit tss entry takes 16 bytes so write upper 8 bytes into next slot
    high_descriptor[0] = (base >> 32) & 0xFFFFFFFF;
    high_descriptor[1] = 0;

    uint8_t* tss_bytes = (uint8_t*)&tss_entry; // clear out the tss memory block
    for (uint32_t i = 0; i < sizeof(tss_entry); i++) {
        tss_bytes[i] = 0;
    }

    tss_entry.rsp0 = esp0; // store kernel mode stack pointer into tss structure 
    (void)ss0; // ignore unused ss0 register parameter
}

void initGdt() { // set gdt init function
    gdt_ptr.limit = (sizeof(gdt_entry_struct) * 6) - 1; // define limit parameter size
    gdt_ptr.base  = (uint64_t)&gdt_entries; // store pointer reference address

    setGdtGate(0, 0, 0, 0, 0); // null descriptor segment
    setGdtGate(1, 0, 0xFFFFF, 0x9A, 0xA0); // kernel code segment descriptor flag set to 64 bit code
    setGdtGate(2, 0, 0xFFFFF, 0x92, 0xA0); // kernel data segment descriptor
    setGdtGate(3, 0, 0xFFFFF, 0xFA, 0xA0); // user code segment descriptor 
    setGdtGate(4, 0, 0xFFFFF, 0xF2, 0xA0); // user data segment descriptor

    writeTSS(5, 0x10, 0x90000); // write tss info across slots 5 and 6

    gdt_flush((uint64_t)&gdt_ptr); // flush out the gdt
    tss_flush(); // load tss register
}
