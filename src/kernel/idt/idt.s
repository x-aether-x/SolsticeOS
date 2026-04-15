[extern interrupt_handler]
[global idt_flush]

%macro ISR_NOERRCODE 1 ; macro for no error code
    [global isr%1]
    isr%1:
        push 0              ; fake error code as it doesnt return one
        push %1             ; interrupt number
        jmp isr_common_stub
%endmacro

%macro ISR_ERRCODE 1 ; macro for a cpu error code
    [global isr%1]
    isr%1:
        ; error code automatically pushed by cpu
        push %1             ; interrupt number
        jmp isr_common_stub
%endmacro

ISR_NOERRCODE 0 ; Divide Error
ISR_NOERRCODE 1 ; Debug Exception
ISR_NOERRCODE 2 ; Nonmaskable External Interrupt
ISR_NOERRCODE 3 ; Breakpoint
ISR_NOERRCODE 4 ; Overflow
ISR_NOERRCODE 5 ; BOUND range exeeded
ISR_NOERRCODE 6 ; Invalid Opcode (Undefined Opcode) 
ISR_NOERRCODE 7 ; Device Not Available (No Math Coprocessor)
ISR_ERRCODE 8 ; Double Fault
ISR_NOERRCODE 9 ; Coprocessor Segment Overrun (reserved) 
ISR_ERRCODE 10 ; Invalid TSS
ISR_ERRCODE 11 ; Segment not present
ISR_ERRCODE 12 ; Stack-Segment Fault
ISR_ERRCODE 13 ; General Protection
ISR_ERRCODE 14 ; Page Fault
ISR_NOERRCODE 15 ; Reserved for intel. Do not use. 
ISR_NOERRCODE 16 ; x87 FPU Floating-Point Error (Math Fault) 
ISR_ERRCODE 17 ; Alignment Check
ISR_ERRCODE 18 ; Machine Check 
ISR_NOERRCODE 19 ; SIMD Floating-Point Exception
ISR_NOERRCODE 20 ; Virtualization Exception
ISR_ERRCODE 21 ; Control Protection Exception 

; reserved for cpu
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

; external interrupts

%assign i 32
%rep 224
    ISR_NOERRCODE i
%assign i i+1
%endrep

isr_common_stub:
    pusha                ; save registers
    
    mov ax, ds           ; save lower 16 bits of data segment
    push eax

    mov ax, 0x10         ; load kernel data segment 
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax


    push esp
    call interrupt_handler ; go into c++ code 
    add esp, 4

    pop eax              ; original data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa                 ; restore registers
    add esp, 8           ; cleans up err code an 
    iret                 ; return

idt_flush:
    mov eax, [esp + 4]  ; idt structure pointer (from stack)
    lidt [eax]          ; load idt pointer into cpu
    ret