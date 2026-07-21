#include "printf.h"
#include "utils.h"
#include "console.h"
#include <stdint.h>
#include <stddef.h>

static void print_uint(uint64_t val, int base, bool upper) {
    char buf[64];
    int i = 0;
    const char* digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    if (val == 0) {
        console_putc('0');
        return;
    }
    while (val > 0) {
        buf[i++] = digits[val % base];
        val /= base;
    }
    while (--i >= 0) console_putc(buf[i]);
}

static void print_int(int64_t val) {
    if (val < 0) {
        console_putc('-');
        val = -val;
    }
    print_uint((uint64_t)val, 10, false);
}

static void print_ptr(uint64_t val) {
    console_puts("0x");
    print_uint(val, 16, false);
}

void kputc(char c) {
    console_putc(c);
}

void kputs(const char* s) {
    console_puts(s);
}

extern "C" int printf(const char* format, ...) {
    console_set_color(0x0F, 0x00); // printf always default white on black
    va_list args;
    va_start(args, format);

    for (const char* p = format; *p != '\0'; p++) {
        if (*p != '%') {
            console_putc(*p);
            continue;
        }

        p++;
        if (*p == '\0') break;

        switch (*p) {
            case 'c': console_putc((char)va_arg(args, int)); break;
            case 'd': print_int(va_arg(args, int)); break;
            case 'i': print_int(va_arg(args, int)); break;
            case 'u': print_uint(va_arg(args, unsigned int), 10, false); break;
            case 'x': print_uint(va_arg(args, unsigned int), 16, false); break;
            case 'X': print_uint(va_arg(args, unsigned int), 16, true); break;
            case 'p': print_ptr(va_arg(args, uintptr_t)); break;
            case 's': console_puts(va_arg(args, const char*)); break;
            case '%': console_putc('%'); break;
            default: console_putc('%'); console_putc(*p); break;
        }
    }

    va_end(args);
    return 0;
}

extern "C" void kprintf(const char* format, ...) {
    console_set_color(0x0F, 0x00);
    va_list args;
    va_start(args, format);

    for (const char* p = format; *p != '\0'; p++) {
        if (*p != '%') {
            console_putc(*p);
            continue;
        }

        p++;
        if (*p == '\0') break;

        switch (*p) {
            case 'c': console_putc((char)va_arg(args, int)); break;
            case 'd': print_int(va_arg(args, int)); break;
            case 'i': print_int(va_arg(args, int)); break;
            case 'u': print_uint(va_arg(args, unsigned int), 10, false); break;
            case 'x': print_uint(va_arg(args, unsigned int), 16, false); break;
            case 'X': print_uint(va_arg(args, unsigned int), 16, true); break;
            case 'p': print_ptr(va_arg(args, uintptr_t)); break;
            case 's': console_puts(va_arg(args, const char*)); break;
            case '%': console_putc('%'); break;
            default: console_putc('%'); console_putc(*p); break;
        }
    }

    va_end(args);
}