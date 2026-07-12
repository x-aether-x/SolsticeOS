#include <ext2.h>
#include <utils.h>
#include <printf.h>

static ext2_superblock g_sb;
static uint32_t g_block_size;

bool ext2_init(uint32_t start_lba, uint16_t port) {
    uint8_t buffer[1024];
    read_disk(start_lba + 2, 2, (uintptr_t)buffer, port);

    ext2_superblock* sb = (ext2_superblock*)buffer;

    if (sb->s_magic != EXT2_MAGIC) {
        printf("Error: Not an ext2 filesystem. Magic: %x\n", sb->s_magic);
        return false;
    }

    g_sb = *sb;
    g_block_size = 1024 << g_sb.s_log_block_size;

    printf("Ext2 Initialized. Block size: %d bytes\n", g_block_size);
    return true;
}

void ext2_read_block(uint32_t block_num, void* buffer, uint16_t port) {
    uint32_t sectors_per_block = g_block_size / 512;
    uint32_t lba = block_num * sectors_per_block;
    
    read_disk(lba, sectors_per_block, (uintptr_t)buffer, port);
}

uint32_t get_inode_table(uint16_t port) {
    uint8_t buffer[512];
    ext2_read_block(2, buffer, port);
    printf("DEBUG: First 4 bytes of BGDT block: %x\n", *(uint32_t*)buffer);
    struct ext2_bg_descriptor *bg_desc = (struct ext2_bg_descriptor *)buffer;

    printf("The inode table starts at block: %u\n", bg_desc->bg_inode_table);

    return bg_desc->bg_inode_table;
}

void list_directory(uint32_t block_num, uint16_t port) {
    uint8_t buffer[1024];
    ext2_read_block(block_num, buffer, port);

    uint32_t offset = 0;
    while (offset < g_block_size) {
        ext2_dir_entry* entry = (ext2_dir_entry*)(buffer + offset);
        
        // manually print as its not null terminated
        printf("File: ");
        for(int i = 0; i < entry->name_len; i++) {
            printf("%c", entry->name[i]);
        }
        printf(" (Inode: %u)\n", entry->inode);

        if (entry->rec_len == 0) break; // shouldnt happen but just in case
        offset += entry->rec_len;
    }
}

void read_root_inode(uint16_t port) {
    if (!ext2_init(0, port)) {
        printf("Failed to initialize Ext2!\n");
        return; 
    }

    uint32_t table_block = get_inode_table(port);
    uint8_t buffer[1024];
    ext2_read_block(table_block, buffer, port);

    // inode 2 at index 1
    // index 1 is bad block index 2 is root in ext2
    // offset is (inode_number - 1) * 128
    ext2_inode* root_inode = (ext2_inode*)(buffer + (2 - 1) * 128);

    printf("Root Inode found! Size: %u bytes\n", root_inode->i_size);
    printf("Root Inode Block 0: %u\n", root_inode->i_block[0]);

    list_directory(root_inode->i_block[0], port);
}