# SolsticeOS

A custom bare-bones operating system running on 32-bit protected mode, complete with a custom bootloader, gdt, idt, keyboard interrupts, stack implementation, and a working console with basic shell commands!
I am using the printf function here  -> [A simple printf implimentation](https://github.com/mpaland/printf/tree/master).

NOTE: It is HIGHLY recommended to run this on linux, as all of these dependencies are extremely annoying to install without a good package manager (e.g. pacman, apt, yay etc.)

### Shell commands:
  Help - Prints a help menu with a list of commands
  
  Echo <TEXT> - Prints a line of text
  
  Clear - Clears the screen
  
  Readdisk <SEGMENT> - Reads the disk and returns a formatted hex_dump of the first 128 bytes of a chosen disk segment
  
>>>>>>> 7b2b85b2a63389decf42220218bdd176e5a7ca04

### Dependencies

[mkisofs](https://wiki.osdev.org/Mkisofs)

[nasm](https://www.nasm.us)

[qemu](https://www.qemu.org)

[i386-elf-gcc](https://gcc.gnu.org/)

[any working vnc - this is mine](https://sourceforge.net/projects/tigervnc/files/latest/download)

Or just run
`yay -Syyu i386-elf-gcc qemu nasm cdrtools` on arch

and `sudo apt update && sudo apt install nasm genisoimage qemu-system-x86` on debian/ubuntu, and build i386-elf-gcc for yourself

### Running the Operating System

To run, clone the repo and run:

`make clean && make all`

`qemu-system-i386 -drive format=raw,file=build/SolsticeOS.bin -serial stdio -vnc :1`

Then use a VNC viewer of choice to view the OS (on port localhost:5901)

Note that if you are going to be running this on a linux distro (NOT WSL), you can run 
`qemu-system-i386 -drive format=raw,file=build/SolsticeOS.bin -serial stdio` and it will simply open a new window, allowing you to skip the VNC viewer install

### [My Documentation](https://github.com/x-aether-x/SolsticeOS/tree/main/docs)



If you want to use my project or some part of it in your project, please link it on your project!
