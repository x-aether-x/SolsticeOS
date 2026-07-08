[bits 64]
[extern kernel_entry]

section .text
global _start
_start:
    ; stack
    mov rsp, 0x70000 
    xor rbp, rbp
    
    mov rdi, 0x9000
    
    call kernel_entry
    
.hang:
    hlt
    jmp .hang