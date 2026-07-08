global gdt_flush
global tss_flush        ; make tss_flush function global for linker

gdt_flush:
    lgdt [rdi]          ; load gdt pointer into cpu

    mov ax, 0x10        ; kernel data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    push 0x08           ; kernel code segment selector
    lea rax, [rel .flush]
    push rax
    retfq               ; far return: pops RIP then CS, reloading CS

.flush:
    ret

tss_flush:
    mov ax, 0x2B        ; load index offset value position for tss descriptor into ax
    ltr ax              ; load task register with tss descriptor value index
    ret                 ; return from tss function
