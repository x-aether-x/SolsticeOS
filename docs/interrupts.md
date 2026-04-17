Essentially, in standard assembly, you have something called interrupts. These run whenever you have an error, or something very specific happens in the hardware. But since I am working with the bare kernel, we don't have access to these interrupts, so we have to create them ourselves.

There are a total of 256 interrupts, all with different purposes.
Interrupts 1-31 are errors, or exceptions
Interrupts 32-256 are hardware interrupts, or user-defined

To make interrupts working, you need three main parts:
    Interrupt Descriptor Table, or IDT (this holds all of the interrupts)
    Interrupt Requests, or IRQs (these are the hardware interrupts, e.g. keyboard input, mouse input, etc.)
    Interrupt Service Routines, or ISRs (this is the code that runs to handle interrupts)

As an example of the Interrupts system:
    Let's say you press a button on the keyboard, for example, the "A" key
    Because there is a interrupt for that in your IDT, it then checks to see if there is an IRQ associated with it.
    And then it finds one (IRQ1), and runs a specific block of code that you write (the ISR)

IRQs I've setup

IRQ1 - Keyboard Interrupts 
    When a button on the keyboard is pressed, it prints to the screen using the vga_print() function.
    It also saves the letter to a variable called buffer_index, which contains the current command being typed into the console.

