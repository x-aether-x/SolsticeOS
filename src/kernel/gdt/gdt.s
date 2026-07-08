global gdt_flush
global tss_flush        ; make tss_flush function global for linker

gdt_flush:
    ; rdi contains first argument address of gdt_ptr structure
    lgdt [rdi]          ; load gdt pointer into cpu

    mov ax, 0x10        ; load kernel data segment selector descriptor
    mov ds, ax          ; reload data segment register
    mov es, ax          ; reload extra segment register
    mov fs, ax
    mov gs, ax
    mov ss, ax          ; reload stack segment register

    push 0x10           ; push data segment selector parameter onto stack
    push rsp            ; push stack pointer address reference onto stack
    pushfq              ; push processor flags state onto stack
    push 0x08           ; push code segment selector descriptor parameter onto stack
    lea rax, [rel .flush] ; load relative label instruction address location into rax
    push rax            ; push target address instruction location onto stack
    iretq               ; far return to flush pipeline execution into long mode

.flush:
    ret                 ; return from flush function

tss_flush:
    mov ax, 0x2B        ; load index offset value position for tss descriptor into ax
    ltr ax              ; load task register with tss descriptor value index
    ret                 ; return from tss function
