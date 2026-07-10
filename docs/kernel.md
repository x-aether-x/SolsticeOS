InitGDT() and InitIDT(), initialises the Interrupt Descriptor Table, and the Global Descriptor Table
remap_pic() allows for the software to take input from the keyboard via interrupts

It then uses vga_print() to print an intro text for the operating system.
The while {0} line is an infinite loop, to keep the kernel running, because otherwise it will turn off.