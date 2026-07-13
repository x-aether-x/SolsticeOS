My Operating system uses an Ext2 filesystem, which is divided up into multiple blocks of space.
Blocks and inodes are divided into things called block groups, which are quite simply put, a group of blocks.

Each block will reserve certain blocks for certain purposes.
An inode is something that is essentially just a link to a specific location in the filesystem (e.g. inode 2 -> root directory)

The first part of implementing a filesystem is to create the superblock.
The superblock is always located at byte 1024, and is always 1024 bytes long
From the superblock we must then find the number of block groups.

We then use this information to fill the superblock with different values in ext2.h.

This is then used to initialise the filesystem.

Next, we have to find and locate the Block Group Descriptor Table (BGDT)
Luckily for us, this is immediatly following the superblock in memory, so we know where to read it from.
We then fill it with data regarding where various important data structures.

We then use the BGDT to find out where the root inode is located in memory, and then from there the fun part begins.

Now that we have initialised the Superblock and BGDT, and found the root inode, we can use this information to list the contents of the root directory using the ls command.
Next, I implemented the mkdir command, which was a little bit more complex to implement, as I had to make a couple new helper functions for it
Finally, I added the cd command, which was surprisingly, the simplest command to implement.

I am still yet to setup interacting with directories that aren't in the current directory (e.g. mkdir test/test1 will create a directory named "test/test1" at the current working location)