[org 0x7c00] ; custom bootloader i made idk if its any good but it works
KERNEL_LOCATION equ 0x10000

mov [BOOT_DISK], dl

xor ax, ax
mov es, ax
mov ds, ax
mov bp, 0x8000
mov sp, bp

mov ax, 0x1000
mov es, ax
mov bx, 0x0000

mov al, 32              ; number of sectors (32*512 = 16384 bytes)
mov ch, 0x00            ; cylinder 0
mov dh, 0x00
mov cl, 0x02
mov dl, [BOOT_DISK]
mov ah, 0x02
int 0x13                ; call bios

jc disk_error           ; check for errors

mov ah, 0x0
mov al, 0x3
int 0x10                ; text mode

mov ah, 0x0
mov al, 0x3
int 0x10

cli
lgdt [GDT_descriptor]
mov eax, cr0
or eax, 1
mov cr0, eax
jmp CODE_SEG:start_protected_mode

disk_error:
    jmp $

GDT_start:
    GDT_null:
        dd 0x0
        dd 0x0

    GDT_code:
        dw 0xffff
        dw 0x0
        db 0x0
        db 0b10011010
        db 0b11001111
        db 0x0

    GDT_data:
        dw 0xffff
        dw 0x0
        db 0x0
        db 0b10010010
        db 0b11001111
        db 0x0

GDT_end:

GDT_descriptor:
    dw GDT_end - GDT_start - 1
    dd GDT_start

CODE_SEG equ GDT_code - GDT_start
DATA_SEG equ GDT_data - GDT_start

[bits 32]
start_protected_mode:
    mov ax, DATA_SEG
	mov ds, ax
	mov ss, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	
	mov ebp, 0x90000 ; 32 bit stack base pointer
	mov esp, ebp

    jmp KERNEL_LOCATION

BOOT_DISK: db 0

times 510-($-$$) db 0
dw 0xaa55
