OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)
SECTIONS
{
    . = 0x100000;
    _kernel_start = .;
    .text : ALIGN(4K) {
        *(.text._start)
        *(.text)
        *(.text.*)
    }
    .data : ALIGN(4K) { *(.data) *(.data.*) }
    .rodata : ALIGN(4K) { *(.rodata) *(.rodata.*) }
    .bss : ALIGN(4K) {
        __bss_start = .;
        *(.bss) *(.bss.*) *(COMMON)
        __bss_end = .;
    }
    _kernel_end = .;
    /DISCARD/ : { *(.note*) *(.comment*) *(.eh_frame*) }
}