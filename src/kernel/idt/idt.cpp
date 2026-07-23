// idt file used to setup the interrupt descriptor table to allow interrupts in my code

#include <stdint.h>
#include "idt.h"
#include "utils.h"
#include "printf.h"
#include "io.h"
#include "console.h"
#include "ext2.h"
#include "timer.h"
#include "task.h"
#include "cursor.h"
#include "gfx.h"

#define MAX_COMMAND_LEN 256

char shell_buffer[MAX_COMMAND_LEN];
int buffer_index = 0;
bool shift_pressed = false;

volatile int mouse_x = 100;
volatile int mouse_y = 100;
volatile uint8_t mouse_buttons = 0;

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
    extern void isr0(), isr1(), isr2(), isr3(), isr4(), isr5(), isr6(), isr7();
    extern void isr8(), isr9(), isr10(), isr11(), isr12(), isr13(), isr14(), isr15();
    extern void isr16(), isr17(), isr18(), isr19(), isr20(), isr21(), isr22(), isr23();
    extern void isr24(), isr25(), isr26(), isr27(), isr28(), isr29(), isr30(), isr31();
    extern void isr32(), isr33(), isr34(), isr35(), isr36(), isr37(), isr38(), isr39();
    extern void isr40(), isr41(), isr42(), isr43(), isr44(), isr45(), isr46(), isr47();
    extern void isr48(), isr49(), isr50(), isr51(), isr52(), isr53(), isr54(), isr55();
    extern void isr56(), isr57(), isr58(), isr59(), isr60();
}

extern "C" void* isr_stub_table[] = {
    (void*)isr0, (void*)isr1, (void*)isr2, (void*)isr3, (void*)isr4, (void*)isr5, (void*)isr6, (void*)isr7,
    (void*)isr8, (void*)isr9, (void*)isr10, (void*)isr11, (void*)isr12, (void*)isr13, (void*)isr14, (void*)isr15,
    (void*)isr16, (void*)isr17, (void*)isr18, (void*)isr19, (void*)isr20, (void*)isr21, (void*)isr22, (void*)isr23,
    (void*)isr24, (void*)isr25, (void*)isr26, (void*)isr27, (void*)isr28, (void*)isr29, (void*)isr30, (void*)isr31,
    (void*)isr32, (void*)isr33, (void*)isr34, (void*)isr35, (void*)isr36, (void*)isr37, (void*)isr38, (void*)isr39,
    (void*)isr40, (void*)isr41, (void*)isr42, (void*)isr43, (void*)isr44, (void*)isr45, (void*)isr46, (void*)isr47,
    (void*)isr48, (void*)isr49, (void*)isr50, (void*)isr51, (void*)isr52, (void*)isr53, (void*)isr54, (void*)isr55,
    (void*)isr56, (void*)isr57, (void*)isr58, (void*)isr59, (void*)isr60
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

    for (int i = 0; i < 61; i++) {
        setIdtGate(i, (uint64_t)isr_stub_table[i], 0x08, 0x8E);
    }

    idt_flush((uint64_t)&idt_ptr);
}

struct registers {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;                                     // pushed manually
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax; // pushed by pusha
    uint64_t int_no, err_code;                       // int number and err code
    uint64_t rip, cs, rflags, rsp, ss;           // pushed by cpu
}__attribute__((packed));


// ------------------- MOUSE DRIVER --------------------

uint8_t mouse_cycle = 0;
int8_t mouse_packet[3];

void mouse_wait(bool write) {
    uint32_t timeout = 100000;
    if (write) {
        while (timeout-- && (inb(PS2_STATUS_PORT) & 0x02));
    } else {
        while (timeout-- && !(inb(PS2_STATUS_PORT) & 0x01));
    }
}

void mouse_write(uint8_t value) {
    mouse_wait(true);
    outb(PS2_CMD_PORT, 0xD4);
    mouse_wait(true);
    outb(PS2_DATA_PORT, value);

    mouse_wait(false);
    inb(PS2_DATA_PORT); 
}


void init_mouse() {
    mouse_wait(true);
    outb(PS2_CMD_PORT, 0xA8);

    // enable interrupts
    mouse_wait(true);
    outb(PS2_CMD_PORT, 0x20);
    uint8_t status = inb(PS2_DATA_PORT) | 2;
    mouse_wait(true);
    outb(PS2_CMD_PORT, 0x60);
    mouse_wait(true);
    outb(PS2_DATA_PORT, status);

    mouse_write(0xF6); // default settings
    mouse_write(0xF4); // streaming
}



extern "C" void interrupt_handler(struct registers* regs) {
    // printf("Received Interrupt: %d\n", (int)regs->int_no);
    if (regs->int_no >= 0x28) outb(0xA0, 0x20);
    outb(0x20, 0x20); 

        if (regs->int_no == 44) {
        uint8_t status = inb(PS2_STATUS_PORT);
        if (!(status & 0x20)) return; // ensure mouse data is being sent

        uint8_t data = inb(PS2_DATA_PORT);

        // if all 3 bits arent set, stream is desynced
        if (mouse_cycle == 0 && !(data & 0x08)) return;

        mouse_packet[mouse_cycle++] = (int8_t)data;

        if (mouse_cycle == 3) {
            mouse_cycle = 0;

            uint8_t flags = (uint8_t)mouse_packet[0];
            if (flags & 0xC0) return; // x or y overflow - discard packet

            mouse_buttons = flags & 0x07;

            mouse_x += mouse_packet[1];
            mouse_y -= mouse_packet[2];

            int max_x = gfx_get_width() - 1;
            int max_y = gfx_get_height() - 1;
            if (mouse_x < 0) mouse_x = 0;
            if (mouse_y < 0) mouse_y = 0;
            if (mouse_x > max_x) mouse_x = max_x;
            if (mouse_y > max_y) mouse_y = max_y;
        }
    } 

    if (regs->int_no == 32) {
        timer_ticks++;
        if (timer_ticks % 1 == 0) schedule();
        // print every 1000 ticks (1 second)
        // if (timer_ticks % 1000 == 0) {
            // printf("Uptime: %d seconds\n", (int)(timer_ticks / 1000));
        // }
    }

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
            
            vga_print("\n", 0x0F, 0x00);
            print_prompt(); // colored path + $ (printf would force it all white)
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