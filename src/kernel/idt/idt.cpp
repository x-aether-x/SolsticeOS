// idt file used to setup the interrupt descriptor table to allow interrupts in my code

#include <stdint.h>
#include "idt.h"
#include "utils.h"
#include "printf.h"
#include "io.h"
#include "console.h"

#define MAX_COMMAND_LEN 256

char shell_buffer[MAX_COMMAND_LEN];
int buffer_index = 0;
bool shift_pressed = false;

static const char scancodes_ascii[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',   /* Backspace */
  '\t', /* Tab */
  'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',           /* Enter key */
    0,  /* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0,           /* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0,                 /* Right shift */
  '*',
    0,  /* Alt */
  ' ',  /* Space bar */
    0,  /* Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
  '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
  '+',
    0,  /* 79 - End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0, 
};

static const char scancodes_ascii_shifted[128] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,
    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, '-', 0, 0, 0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


extern "C" {
    idt_entry_struct idt_entries[256];
    idt_ptr_struct idt_ptr;

    void idt_flush(uint64_t);

    extern void isr0(); // okay this isnt even gonnna be that hard
    extern void isr1(); // writing ts 256 times is gonna be easyy
    extern void isr2(); // trust
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
    extern void isr33(); // i just wrote all 256 and realised i didnt even need all of them
}

extern "C" void* isr_stub_table[] = {
    (void*)isr0, (void*)isr1, (void*)isr2, (void*)isr3, (void*)isr4, (void*)isr5, (void*)isr6, (void*)isr7,
    (void*)isr8, (void*)isr9, (void*)isr10, (void*)isr11, (void*)isr12, (void*)isr13, (void*)isr14, (void*)isr15, (void*)isr16, (void*)isr17, (void*)isr18, (void*)isr19, (void*)isr20, (void*)isr21, (void*)isr22, (void*)isr23,
    (void*)isr24, (void*)isr25, (void*)isr26, (void*)isr27, (void*)isr28, (void*)isr29, (void*)isr30, (void*)isr31, (void*)isr32, (void*)isr33
};

extern "C" void setIdtGate(uint8_t vector, uint64_t handler, uint16_t sel, uint8_t flags) {
    idt_entry_struct* descriptor = &idt_entries[vector];

    descriptor->isr_low    = (uint64_t)handler & 0xFFFF;
    descriptor->selector   = sel; 
    descriptor->ist        = 0;
    descriptor->flags      = flags; 
    descriptor->isr_mid    = ((uint64_t)handler >> 16) & 0xFFFF;
    descriptor->isr_high   = ((uint64_t)handler >> 32) & 0xFFFFFFFF;
    descriptor->zero   = 0;
}
 

void initIdt() {
    idt_ptr.limit = (sizeof(idt_entry_struct) * 256) - 1;
    idt_ptr.base = (uint64_t)idt_entries;

    for (int i = 0; i < 34; i++) { // write isr stubs to the idt
        setIdtGate(i, (uint64_t)isr_stub_table[i], 0x08, 0x8E);
    }
    setIdtGate(33, (uint64_t)isr33, 0x08, 0x8E); // load keys manually


    idt_flush((uint64_t)&idt_ptr);
}

struct registers {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;                                     // pushed manually
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax; // pushed by pusha
    uint64_t int_no, err_code;                       // int number and err code
    uint64_t rip, cs, rflags, rsp, ss;           // pushed by cpu
}__attribute__((packed));


extern "C" void interrupt_handler(struct registers* regs) {
    // printf("Received Interrupt: %d\n", (int)regs->int_no);
    if (regs->int_no >= 0x28) outb(0xA0, 0x20);
    outb(0x20, 0x20); 

    if (regs->int_no == 0x21) {
        uint8_t scancode = inb(0x60);

        if (scancode == 0x2A || scancode == 0x36) {
            shift_pressed = true;
            return;
        }
        if (scancode == (0x2A + 0x80) || scancode == (0x36 + 0x80)) {
            shift_pressed = false;
            return;
        }

        if (scancode & 0x80) return;

        char c = shift_pressed ? scancodes_ascii_shifted[scancode] : scancodes_ascii[scancode];

        if (c == '\n') {
            shell_buffer[buffer_index] = '\0';
            vga_print("\n", 0xFF, 0x00);
            
            execute_command(shell_buffer);
            
            vga_print("\n$ ", 0x02, 0x00);
            buffer_index = 0;
            shell_buffer[0] = '\0';
        } 
        else if (c == '\b') {
            if (buffer_index > 0) {
                console_backspace();
                buffer_index--;
                shell_buffer[buffer_index] = '\0';
            }
        } 
        else if (c != 0 && buffer_index < MAX_COMMAND_LEN - 1) {
            console_putc(c); 
            shell_buffer[buffer_index++] = c;
            shell_buffer[buffer_index] = '\0';
        }
    }
} 