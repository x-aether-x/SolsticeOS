OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)
SECTIONS
{
    . = 0x100000;
    .text : ALIGN(4K) {
        *(.text._start)
        *(.text)
    }
    .data : ALIGN(4K) { *(.data) }
    .rodata : ALIGN(4K) { *(.rodata) }
    .bss : ALIGN(4K) { *(.bss) }
}