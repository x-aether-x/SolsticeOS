# SolsticeOS

A custom bare-bones operating system running on 32-bit protected mode, complete with a custom bootloader, gdt, and stack implementation
I am using the printf function here  -> [A simple printf implimentation](https://github.com/mpaland/printf/tree/master).


### Dependencies

[mkisofs](https://wiki.osdev.org/Mkisofs)

[nasm](https://www.nasm.us)

[qemu](https://www.qemu.org)

[i386-elf-gcc](https://gcc.gnu.org/)

### Running the Operating System

To run, download the files and run:

make

qemu-system-i386 -drive format=raw,file=build/SolsticeOS.iso -serial stdio

If you want to use my project or some part of it in your project, please link it on your project
