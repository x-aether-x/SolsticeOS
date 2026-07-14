#include <ext2.h>
#include <utils.h>
#include <printf.h>
#include "io.h"

static ext2_superblock g_sb;
static uint32_t g_block_size;

#define EXT2_S_IFDIR 0x4000
#define EXT2_FT_DIR  2

#define SECTORS_PER_BLOCK 2

#define INODE_SIZE 128
#define INODES_PER_BLOCK (1024 / INODE_SIZE)

static bool g_ext2_initialized = false;

uint32_t g_current_dir = 2; 
char g_current_path[256] = "/";

uint32_t BG_BLOCK_BITMAP_BLOCK = 0;
uint32_t BG_INODE_BITMAP_BLOCK = 0;
uint32_t BG_INODE_TABLE_BLOCK = 0;

bool ext2_init(uint32_t start_lba, uint16_t port) {
    if (g_ext2_initialized) return true; // already done

    uint8_t buffer[1024];
    read_disk(2, 2, (uintptr_t)buffer, port); 
    ext2_superblock* sb = (ext2_superblock*)buffer;

    io_wait(port);
 

    if (sb->s_magic != EXT2_MAGIC) return false;

    g_sb = *sb;
    g_block_size = 1024 << g_sb.s_log_block_size;

    ext2_read_block(2, buffer, port);
    ext2_bg_descriptor* bgdt = (ext2_bg_descriptor*)buffer;

    BG_BLOCK_BITMAP_BLOCK = bgdt->bg_block_bitmap;
    BG_INODE_BITMAP_BLOCK = bgdt->bg_inode_bitmap;
    BG_INODE_TABLE_BLOCK  = bgdt->bg_inode_table;

    g_ext2_initialized = true;
    return true;
}

void ext2_read_block(uint32_t block_num, void* buffer, uint16_t port) {
    uint32_t sectors_per_block = g_block_size / 512;
    uint32_t lba = (block_num * sectors_per_block) + 0;
    
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

uint32_t ext2_traverse_path(const char* path, uint16_t port, uint32_t* out_parent = nullptr, char* out_name = nullptr) { // traverses paths (this is why we clash)
    uint32_t curr_inode = (path[0] == '/') ? 2 : g_current_dir;

    if (strcmp(path, "/") == true) {
        if (out_parent) *out_parent = 2;
        if (out_name) strcpy(out_name, "/");
        return 2; 
    }

    char comp[256];
    int i = 0;
    if (path[i] == '/') i++;

    while (path[i] != '\0') {
        int j = 0;
        while (path[i] != '\0' && path[i] != '/') {
            comp[j++] = path[i++];
        }
        comp[j] = '\0';
        while (path[i] == '/') i++;

        bool is_last = (path[i] == '\0');
        if (is_last && out_parent && out_name) {
            *out_parent = curr_inode;
            strcpy(out_name, comp);
        }

        uint8_t type = 0;
        uint32_t next_inode = ext2_find_in_dir(curr_inode, comp, &type, port);
        if (is_last) {
            return next_inode; 
        }
        if (next_inode == 0 || type != EXT2_FT_DIR) {
            return 0; 
        }

        curr_inode = next_inode;
    }
    return curr_inode;
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

void ext2_ls_root(uint16_t port) {
    if (!ext2_init(0, port)) {
        vga_print("Error: Filesystem not initialized!\n", 0xFF, 0x00);
        return;
    }

    uint8_t* inode_buffer = (uint8_t*)0x20000; 
    uint8_t* dir_buffer   = (uint8_t*)0x21000; 

    read_disk(BG_INODE_TABLE_BLOCK * 2, 2, (uintptr_t)inode_buffer, port); // block -> LBA (1K blocks = 2 sectors)
    ext2_inode* root_inode = (ext2_inode*)(inode_buffer + 128); // inode 2 = index 1, 128-byte inodes
    uint32_t dir_block = root_inode->i_block[0];

    // switch block id to lba (multiply by two)
    uint32_t dir_lba = dir_block * 2;
    read_disk(dir_lba, 2, (uintptr_t)dir_buffer, port);

    uint32_t offset = 0;
    
    vga_print("Root Directory Contents:\n", 0x03, 0x00);

    // loop through the block
    while (offset < 1024) {
        ext2_dir_entry* entry = (ext2_dir_entry*)(dir_buffer + offset);

        // if inode is 0 its not too bad but still pretty bad
        // if rec_len is 0 youre frapped
        if (entry->inode == 0 || entry->rec_len == 0) {
            break; 
        }

        for (int i = 0; i < entry->name_len; i++) {
            int color = (entry->file_type == 2) ? 0x09 : 0x0F; // blue is dirs, white is files
            
            char c[2] = { entry->name[i], '\0' };
            vga_print(c, color, 0x00);
        }
        
        vga_print("   ", 0xFF, 0x00); // spaces between filenames
        offset += entry->rec_len;
    }
    
    vga_print("\n", 0xFF, 0x00);
}

static bool name_matches(ext2_dir_entry* e, const char* name) {
    int len = strlen(name);
    if (e->name_len != len) return false;
    for (int i = 0; i < len; i++) {
        if (e->name[i] != name[i]) return false;
    }
    return true;
}

// returns inode number of 'name' inside directory dir_inode_num, 0 if not found
uint32_t ext2_find_in_dir(uint32_t dir_inode_num, const char* name, uint8_t* out_type, uint16_t port) {
    ext2_inode dir_inode;
    ext2_read_inode(dir_inode_num, &dir_inode, port);

    uint8_t block_buffer[1024];
    ext2_read_block(dir_inode.i_block[0], block_buffer, port);

    uint32_t offset = 0;
    while (offset < 1024) {
        ext2_dir_entry* e = (ext2_dir_entry*)(block_buffer + offset);
        if (e->inode == 0 || e->rec_len == 0) break;
        if (name_matches(e, name)) {
            if (out_type) *out_type = e->file_type;
            return e->inode;
        }
        offset += e->rec_len;
    }
    return 0;
}

bool ext2_cd(const char* path, uint16_t port) { // waay simpler then before cos i implemented the path traversla function
    if (!ext2_init(0, port)) {
        vga_print("Error: Filesystem not initialized!\n", 0xFF, 0x00);
        return false;
    }
    uint32_t target_inode = ext2_traverse_path(path, port);
    
    if (target_inode == 0) {
        vga_print("cd: no such file or directory\n", 0xFF, 0x00);
        return false;
    }
    ext2_inode check_inode;
    ext2_read_inode(target_inode, &check_inode, port);
    if ((check_inode.i_mode & EXT2_S_IFDIR) == 0) {
        vga_print("cd: not a directory\n", 0xFF, 0x00);
        return false;
    }

    g_current_dir = target_inode;

    if (strcmp(path, "/") == true) {
        strcpy(g_current_path, "/");
    } else if (strcmp(path, "..") == true) {
        size_t len = strlen(g_current_path);
        size_t last_slash_idx = 0;
        for (size_t i = 0; i < len; i++) {
            if (g_current_path[i] == '/') last_slash_idx = i;
        }
        if (last_slash_idx == 0) strcpy(g_current_path, "/");
        else g_current_path[last_slash_idx] = '\0';
    } else if (strcmp(path, ".") != true) {
        if (path[0] == '/') {
            strcpy(g_current_path, path);
        } else {
            if (strcmp(g_current_path, "/") != true) {
                strcat(g_current_path, "/");
            }
            strcat(g_current_path, path);
        }
    }

    return true;
}



// lists the current directory but can also list other directories now yay
void ext2_ls(const char* path, uint16_t port) {
    if (!ext2_init(0, port)) {
        vga_print("Error: Filesystem not initialized!\n", 0xFF, 0x00);
        return;
    }

    uint32_t target_inode = g_current_dir;
    if (path && path[0] != '\0') {
        target_inode = ext2_traverse_path(path, port);
        if (target_inode == 0) {
            vga_print("ls: cannot access path: No such file or directory\n", 0xFF, 0x00);
            return;
        }
    }

    ext2_inode dir_inode;
    ext2_read_inode(target_inode, &dir_inode, port);

    uint8_t block_buffer[1024];
    ext2_read_block(dir_inode.i_block[0], block_buffer, port);

    uint32_t offset = 0;
    while (offset < 1024) {
        ext2_dir_entry* entry = (ext2_dir_entry*)(block_buffer + offset);
        if (entry->inode == 0 || entry->rec_len == 0) break;

        for (int i = 0; i < entry->name_len; i++) {
            int color = (entry->file_type == 2) ? 0x09 : 0x0F;
            char c[2] = { entry->name[i], '\0' };
            vga_print(c, color, 0x00);
        }
        vga_print("   ", 0xFF, 0x00);
        offset += entry->rec_len;
    }
    vga_print("\n", 0xFF, 0x00);
}

void ext2_write_block(uint32_t block, void* buffer, uint16_t port) {
    uint32_t lba = block * SECTORS_PER_BLOCK;
    write_disk(lba, SECTORS_PER_BLOCK, (uintptr_t)buffer, port);
}

void ext2_write_inode(uint32_t inode_num, ext2_inode* inode_data, uint16_t port) {
    uint32_t index = inode_num - 1; 
    uint32_t block_offset = index / INODES_PER_BLOCK;
    uint32_t byte_offset_in_block = (index % INODES_PER_BLOCK) * INODE_SIZE;
    uint32_t target_block = BG_INODE_TABLE_BLOCK + block_offset;
    uint8_t block_buffer[1024];

    ext2_read_block(target_block, block_buffer, port);

    memcpy(block_buffer + byte_offset_in_block, inode_data, sizeof(ext2_inode));

    ext2_write_block(target_block, block_buffer, port);
}

uint32_t ext2_alloc_block(uint16_t port) {
    uint8_t bitmap[1024];
    ext2_read_block(BG_BLOCK_BITMAP_BLOCK, bitmap, port);

    for (int i = 0; i < 1024; i++) {
        if (bitmap[i] != 0xFF) {
            for (int bit = 0; bit < 8; bit++) {
                if (!(bitmap[i] & (1 << bit))) {
                    
                    // mark as used
                    bitmap[i] |= (1 << bit); 
                    ext2_write_block(BG_BLOCK_BITMAP_BLOCK, bitmap, port);
                    
                    return (i * 8) + bit + 1; 
                }
            }
        }
    }
    return 0;
}

uint32_t ext2_alloc_inode(uint16_t port) {
    uint8_t bitmap[1024];
    ext2_read_block(BG_INODE_BITMAP_BLOCK, bitmap, port);

    for (int i = 0; i < 1024; i++) {
        if (bitmap[i] != 0xFF) {
            for (int bit = 0; bit < 8; bit++) {
                if (!(bitmap[i] & (1 << bit))) {
                    
                    bitmap[i] |= (1 << bit); 
                    ext2_write_block(BG_INODE_BITMAP_BLOCK, bitmap, port);
                    
                    return (i * 8) + bit + 1; 
                }
            }
        }
    }
    return 0;
}

bool ext2_link_dir_entry(uint32_t parent_inode_num, const char* name, uint32_t new_inode, uint8_t type, uint16_t port) {
    ext2_inode parent_inode; 
    ext2_read_inode(parent_inode_num, &parent_inode, port);

    uint32_t dir_block = parent_inode.i_block[0];
    

    uint8_t block_buffer[1024];

    ext2_read_block(dir_block, block_buffer, port);
    
    uint32_t offset = 0;
    int loop_guard = 0;
    ext2_dir_entry* entry = nullptr;

    while (offset < 1024 && loop_guard < 100) {
        ext2_dir_entry* entry = (ext2_dir_entry*)(block_buffer + offset);

        if (entry->rec_len == 0) {
            printf("CRITICAL: Corrupt rec_len at offset %u\n", offset);
            return false;
        }

        uint16_t true_size = 8 + entry->name_len;
        if (true_size % 4 != 0) {
            true_size += (4 - (true_size % 4));
        }

        if (entry->rec_len > true_size && (offset + entry->rec_len == 1024)) {

            uint16_t old_rec_len = entry->rec_len;
            entry->rec_len = true_size;
            
            offset += true_size;
            
            ext2_dir_entry* new_entry = (ext2_dir_entry*)(block_buffer + offset);
            new_entry->inode = new_inode;
            new_entry->file_type = type;
            new_entry->name_len = strlen(name);
            
            new_entry->rec_len = old_rec_len - true_size; 
            memcpy(new_entry->name, name, new_entry->name_len);

            ext2_write_block(dir_block, block_buffer, port);
            return true;
        }

        offset += entry->rec_len;
        loop_guard++;
    }

    vga_print("Error: Parent directory block is full.\n", 0xFF, 0x00);
    return false;
}

void ext2_read_inode(uint32_t inode_num, ext2_inode* buffer, uint16_t port) {
    uint32_t index = inode_num - 1;
    uint32_t block_offset = index / INODES_PER_BLOCK;
    uint32_t byte_offset_in_block = (index % INODES_PER_BLOCK) * INODE_SIZE;
    uint32_t target_block = BG_INODE_TABLE_BLOCK + block_offset;
    

    uint8_t block_buffer[1024];
    ext2_read_block(target_block, block_buffer, port);
    memcpy(buffer, block_buffer + byte_offset_in_block, sizeof(ext2_inode));

}

bool ext2_mkdir(const char* path, uint16_t port) {
    if (!ext2_init(0, port)) {
        vga_print("Error: Filesystem not initialized!\n", 0xFF, 0x00);
        return false;
    }

    uint32_t parent_inode_num = 0;
    char dir_name[256];

    uint32_t existing_inode = ext2_traverse_path(path, port, &parent_inode_num, dir_name);
    
    if (existing_inode != 0) {
        vga_print("mkdir: cannot create directory: File exists\n", 0xFF, 0x00);
        return false;
    }
    if (parent_inode_num == 0) {
        vga_print("mkdir: cannot create directory: No such parent directory\n", 0xFF, 0x00);
        return false;
    }

    uint32_t new_inode_num = ext2_alloc_inode(port);
    if (new_inode_num == 0) {
        vga_print("mkdir: No free inodes.\n", 0xFF, 0x00);
        return false;
    }

    uint32_t new_block_num = ext2_alloc_block(port);
    if (new_block_num == 0) {
        vga_print("mkdir: No free blocks.\n", 0xFF, 0x00);
        return false;
    }

    ext2_inode new_inode;
    memset(&new_inode, 0, sizeof(ext2_inode));
    
    new_inode.i_mode = EXT2_S_IFDIR | 0x01ED; 
    new_inode.i_size = 1024; 
    new_inode.i_blocks = 2;  
    new_inode.i_links_count = 2; 
    new_inode.i_block[0] = new_block_num; 

    ext2_write_inode(new_inode_num, &new_inode, port);

    uint8_t block_buffer[1024];
    memset(block_buffer, 0, 1024);

    uint32_t offset = 0;

    // .
    ext2_dir_entry* dot_entry = (ext2_dir_entry*)(block_buffer + offset);
    dot_entry->inode = new_inode_num;
    dot_entry->name_len = 1;
    dot_entry->file_type = EXT2_FT_DIR;
    dot_entry->name[0] = '.';
    dot_entry->rec_len = 12; 
    offset += dot_entry->rec_len;

    //..
    ext2_dir_entry* dotdot_entry = (ext2_dir_entry*)(block_buffer + offset);
    dotdot_entry->inode = parent_inode_num;
    dotdot_entry->name_len = 2;
    dotdot_entry->file_type = EXT2_FT_DIR;
    dotdot_entry->name[0] = '.';
    dotdot_entry->name[1] = '.';
    dotdot_entry->rec_len = 1024 - offset; 

    ext2_write_block(new_block_num, block_buffer, port);

    bool linked = ext2_link_dir_entry(parent_inode_num, dir_name, new_inode_num, EXT2_FT_DIR, port);
    if (!linked) {
        vga_print("mkdir: Failed to link to parent.\n", 0xFF, 0x00);
        return false;
    }

    vga_print("Directory created successfully!\n", 0x0A, 0x00);
    return true;
}