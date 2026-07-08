[extern interrupt_handler]
[global idt_flush]

%macro ISR_NOERRCODE 1
    [global isr%1]
    isr%1:
        cli
        push qword 0         ; 64-bit fake error code
        push qword %1        ; 64-bit interrupt number
        jmp isr_common_stub
%endmacro

%macro ISR_ERRCODE 1
    [global isr%1]
    isr%1:
        cli
        push qword %1        ; 64-bit interrupt number
        jmp isr_common_stub
%endmacro

ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE 8
ISR_NOERRCODE 9
ISR_ERRCODE 10
ISR_ERRCODE 11
ISR_ERRCODE 12
ISR_ERRCODE 13
ISR_ERRCODE 14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_ERRCODE 17
ISR_ERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_ERRCODE 21

ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

%assign i 32
%rep 224
    ISR_NOERRCODE i
%assign i i+1
%endrep

isr_common_stub:
    cld
    
    ; Replace pusha with individual 64-bit register saves
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    ; Under the 64-bit System V ABI, the first function argument goes into RDI
    mov rdi, rsp
    
    call interrupt_handler

    ; Restore 64-bit registers individually
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    add rsp, 16          ; Cleans up the 64-bit error code and interrupt number
    iretq                ; 64-bit interrupt return instruction

idt_flush:
    ; 64-bit calling convention passes the first parameter in rdi
    lidt [rdi]
    ret
