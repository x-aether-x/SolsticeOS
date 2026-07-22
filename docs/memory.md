This is probably the worst part of making my operating system so far, and was to say the least.. horrible.

So, basically I needed memory management so that I could eventually implement a malloc() and kmalloc() functions.
But, to get there I first had to initialise and create my PMM

## The PMM

So what is the PMM? Essentially, the PMM, or physical memory manager, creates a stack of data, and allocates 4KB "pages" to certain spots in memory.
We do this using a linked list, where each page has a pointer inside of it, pointing to the next page.

### How does it work?

In a nutshell, the PMM takes in a memory map, provided by the bootloader at 0x80000, and filters through it to find standard usable memory.
It then splits this memory into 4KB "pages", carefully avoiding the first 2 MB of data, as they contain important information, which would go very badly if we tried to sort through those.

## The VMM

The VMM, also called the virtual memory manager, controls a system called paging, which involves the allocation of and removal of 4KB pages, previously created using the PMM
It uses 4-level paging, and scans each level using the get_or_create_table() function, which creates system page tables when going through unmapped places in memory, and then zeroes out the newly created pages, to remove any useless accidentall data.
identity_map_range() then maps all of the virtual memory adresses to the physical memory adresses, thus completing the cycle and allowing us to page our operating system.

## The Slab Allocator

The Slab allocator, (the simplest of the three) handles memory allocations under 1024 bytes, and uses the paging system implemented to allocate a number of consecutive pages, called a Slab, and outputs the memory adress.
This is then used in the kmalloc() and kfree() commands, which free up, and allocate any number of memory in the system