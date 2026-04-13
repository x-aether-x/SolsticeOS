[bits 32]
[extern main]

section .text
global _start
_start:
    call main
    hlt
jmp $
