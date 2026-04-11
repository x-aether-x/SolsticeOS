# SolsticeOS

A custom bare-bones operating system running on 32-bit protected mode, complete with a custom bootloader, gdt, and stack implementation
I am using the printf function here  -> [A simple printf implimentation](https://github.com/mpaland/printf/tree/master).

NOTE: It is HIGHLY recommended to run this on linux, as all of these dependencies are extremely annoying to install without a good package manager (e.g. pacman, apt, yay etc.)


### Dependencies

[mkisofs](https://wiki.osdev.org/Mkisofs)

[nasm](https://www.nasm.us)

[qemu](https://www.qemu.org)

[i386-elf-gcc](https://gcc.gnu.org/)

Or just run
`sudo pacman -Syyu i386-elf-gcc qemu nasm mkisofs` on arch

and `sudo apt update && sudo apt install nasm genisoimage qemu-system-x86` on debian/ubuntu

### Running the Operating System

To run, clone the repo and run:

make

qemu-system-i386 -drive format=raw,file=build/SolsticeOS.iso -serial stdio

If you want to use my project or some part of it in your project, please link it on your project
