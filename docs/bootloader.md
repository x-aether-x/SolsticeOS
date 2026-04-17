Imagine that your operating system is a person (I know it sounds a little bit weird, but just trust me on this one)

The operating system's alarm goes off, and it wakes up (power on) 
It then opens up its eyes and has a look where it is (BIOS/MBR)
After that, it gets dressed and ready to go out for the day (Switches to 32-bit Protected Mode)
Finally, it walks out the door to go to work (Jumping to kernel)

This is a simple explanation of how my basic bootloader I coded works.

Technical Stuff:

32-bit protected mode bootloader which loads into VGA text mode

Register dl holds the boot drive id

Memory map:

    0x7C00: Bootloader (temporary)

    0x8000: RAM Map from BIOS

    0x9000: VBE Info Block

    0x10000: Kernel Start

The stack:
    esp: 0x90000