# SolsticeOS

A custom bare-bones operating system running on 32-bit protected mode, complete with a custom bootloader, gdt, idt, keyboard interrupts, stack implementation, and a working console with basic shell commands, and backspace support!
I am using the printf function here  -> [A simple printf implimentation](https://github.com/mpaland/printf/tree/master).

NOTE: It is HIGHLY recommended to run this on linux, as all of these dependencies are extremely annoying to install without a good package manager (e.g. pacman, apt, yay etc.)

### Shell commands:
  Help - Prints a help menu with a list of commands
  
  Echo \<TEXT\> - Prints a line of text
  
  Clear - Clears the screen
  
  Readdisk \<SEGMENT\> - Reads the disk and returns a formatted hex_dump of the first 128 bytes of a chosen disk segment

### Dependencies

[mkisofs](https://wiki.osdev.org/Mkisofs)

[nasm](https://www.nasm.us)

[qemu](https://www.qemu.org)

[i386-elf-gcc](https://gcc.gnu.org/)

[ovmf](https://github.com/tianocore/tianocore.github.io/wiki/OVMF)

[dosfstools](https://github.com/dosfstools/dosfstools)

[mingw-w64](https://www.mingw-w64.org/)

[any working vnc - this is mine](https://sourceforge.net/projects/tigervnc/files/latest/download)



### Running the Operating System

To run, clone the repo and run:

`make clean && make all`

`make run`

Or, to debug with gdb, just run 
`make debug`, which runs the file with -s -S commands

### [My Documentation](https://github.com/x-aether-x/SolsticeOS/tree/main/docs)

If you want to use my project or some part of it in your project, please link it on your project!
