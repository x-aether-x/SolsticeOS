The kernel is the heart of the code. The kernel is what runs everything, and what calls all of the functions from the other files.

The kernel initialises a function named _putchar(), which sends text through a special pin in the motherboard, and is required for the printf implementation to work.

InitGDT() and InitIDT(), initialises the Interrupt Descriptor Table, and the Global Descriptor Table
remap_pic() and outb(0x21, 0xFD) allow for the software to take input from the keyboard via interrupts

It then uses vga_print() to print an intro text for the operating system.
The while {0} line is an infinite loop, to keep the kernel running, because otherwise it will turn off.