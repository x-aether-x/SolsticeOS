Files can get messy sometimes, especially when concerning operating systems, so they need rules.
The global descriptor table, often reffered to as the GDT, is a way of setting those rules. 
It's like a security system that only allows certain parts of the system to access other parts of the system

It has a:
    Kernel code segment, this is where the kernel can write code
    Kernel data segment, which is where the kernel can read code written by anyone, but not edit it
    User code segment, where the user is able to write code
    User data segment, where the user is able to view the code written by anyone, but not edit it

Technical Stuff:

The GDT is a data structure used by Intel x86-family processors to define the characteristics of the various memory areas used during program execution.

It is loaded through the lgdt command with a pointer to a descriptor containing the GDT's size and starting adress

The segments are defined like this:

CODE_SEG (0x08)
DATA_SEG (0x10)

My kernel has full Ring 0 access, which means that it has permissions to read or write anything on the Operating System
