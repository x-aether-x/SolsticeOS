# Boring bootloader I wrote

Boots using UEFI and uses my own efi.h file because apparently gnu-efi and pico-efi both hate me

bootloader.cpp is the bootloader my OS is currently using for UEFI, and boot.asm is my legacy bootloader from when I was booting in BIOS

Technical Stuff:

64-bit UEFI bootloader which loads into protected/long mode with a linear framebuffer

What it actually does:
    Reads and loads kernel.bin into 0x100000
    Stores framebuffer info at 0x9000
    Gets the memory map and calls ExitBootServices
    Casts 0x100000 to a function pointer and executes it, thus starting the kernel