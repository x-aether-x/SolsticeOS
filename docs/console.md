# How the console works

Back when I was using BIOS, this was much easier, as I could just pass a value to a hardware address using VGA Text Mode, and the hardware could do the rest printing characters-wise. But now that I have switched to UEFI, I need to take a different approach.

I have to use a custom font, that knows what each character looks like, and print each character pixel by pixel across the screen instead. This process requires a framebuffer, which I have implemented into my bootloader, and a font. My font of choice was SSFN (scalable screen font). The details for the font are located in includes/FreeSans.sfn if you want to check it out!

I use a function I made called vga_putc() to print characters to the screen using the framebuffer, and if any of those characters exceed the 1024 × 768 pixel grid used by QEMU, then I loop them around.

I use keyboard interrupts (IRQ1) to detect whether or not a key is being pressed, and the current command is stored in a variable called buffer_index.
Whenever the user hits enter, I make it so that the program goes to a newline, and check if the buffer_index is any valid commands, and then if it is, I run the chosen command.

Backspace Implementation:
    Backspaces were pretty easy, all I had to do was mov col back one, and delete the letter from the command buffer. We don't allow backspaces if col <= 2, because then it would be deleting the "$ ", which we don't want deleted


My list of commands is:
    help - Lists commands and displays a help screen
    echo \<TEXT\>- Prints a line of text
    clear - Clears the screen
    readdisk \<SEGMENT\>- Reads a chosen segment of the disk, and returns a hex dump

Help was pretty simple, as I could just use the vga_putc() function to print some simple text

Echo was a little bit different, as instead of comparing the buffer_index to the command letter for letter(using my strcmp function), I had to check if the buffer_index starts with echo, and so I created a new function, starts_with.

Clear was easy enough, I just had to clear the screen character by character, and reset the cursor

Readdisk has definetely by far got to be the most difficult command to implement.
I had to create a whole new, complicated function called read_disk(), which looked through the given LBA, and then had to read the first 128 bytes of the chosen disk LBA, and outputted it in a specific format.

For this format I had to make yet another two functions, print_hex_8bit(), and hex_dump().

hex_dump(), shows the raw data outputted from readdisk, because otherwise, it would print garbage characters, like this: ÃƒÂ©

print_hex_8bit() converts the raw numerical data from hex_dump() into at least slightly readable data, in a hexadecimal format.