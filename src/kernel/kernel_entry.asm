[bits 64]
[extern kernel_entry]
[extern __bss_start]
[extern __bss_end]

section .text
global _start
_start:
    mov rsp, 0x70000
    xor rbp, rbp

    mov rdi, __bss_start
    mov rcx, __bss_end
    sub rcx, rdi
    xor eax, eax
    rep stosb

    mov rdi, 0x9000
    call kernel_entry

.hang:
    hlt
    jmp .hang