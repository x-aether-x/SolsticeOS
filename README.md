# SolsticeOS

A custom bare-bones operating system running on 64-bit protected/long mode, complete with a custom UEFI bootloader, gdt, idt, keyboard interrupts, stack implementation, and a working console with basic shell commands, and backspace support!
I am no longer using the printf function here, as it was no longer working with my recent switch to UEFI [A simple printf implimentation](https://github.com/mpaland/printf/tree/master).

btw i put in alot of effort into this and I'd really appreciate a star as it really does go a long way!

NOTE: It is HIGHLY recommended to run this on linux, as all of these dependencies are extremely annoying to install without a good package manager (e.g. pacman, apt, yay etc.)

### Shell commands:
  help - Prints a help menu with a list of commands
  
  echo \<TEXT\> - Prints a line of text
  
  clear - Clears the screen
  
  readdisk \<SEGMENT\> - Reads the disk and returns a formatted hex_dump of the first 128 bytes of a chosen disk segment
  
  cd \<DIR\> - Changes directory (cd .. to go up, cd / or bare cd for root)

  ls \<DIR\> - Lists the contents of the current working directory by default, but can also view contents of other directories with an optional flag

  mkdir \<DIR_NAME\> - Creates a new directory at a user provided location
  
  sleep \<SECONDS\> - Pauses the kernel for a user specified number of seconds

### Dependencies

[mkisofs](https://wiki.osdev.org/Mkisofs)

[nasm](https://www.nasm.us)

[qemu](https://www.qemu.org)

[i386-elf-gcc](https://gcc.gnu.org/)

[ovmf](https://github.com/tianocore/tianocore.github.io/wiki/OVMF)

[dosfstools](https://github.com/dosfstools/dosfstools)

[xorriso](https://scdbackup.sourceforge.net/xorriso_eng.html)

### Running the Operating System

To run, clone the repo and run:

`make clean && make all`

`make run`

Or, to debug with gdb, just run 
`make debug`, which runs the file with -s -S commands

### [My Documentation](https://github.com/x-aether-x/SolsticeOS/tree/main/docs)

If you want to use my project or some part of it in your project, please link it on your project! 
