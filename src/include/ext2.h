#include <stdint.h>

#define EXT2_MAGIC 0xEF53

#define EXT2_SUPERBLOCK_OFFSET 1024

extern uint32_t g_current_dir;
extern uint32_t BG_BLOCK_BITMAP_BLOCK;
extern uint32_t BG_INODE_BITMAP_BLOCK;
extern uint32_t BG_INODE_TABLE_BLOCK;

struct ext2_superblock {
    uint32_t s_inodes_count;      // Inodes count
    uint32_t s_blocks_count;      // Blocks count
    uint32_t s_r_blocks_count;    // Reserved blocks count
    uint32_t s_free_blocks_count; // Free blocks count
    uint32_t s_free_inodes_count; // Free inodes count
    uint32_t s_first_data_block;  // First Data Block
    uint32_t s_log_block_size;    // Block size
    uint32_t s_log_frag_size;     // Fragment size
    uint32_t s_blocks_per_group;  // # Blocks per group
    uint32_t s_frags_per_group;   // # Fragments per group
    uint32_t s_inodes_per_group;  // # Inodes per group
    uint32_t s_mtime;             // Mount time
    uint32_t s_wtime;             // Write time
    uint16_t s_mnt_count;         // Mount count
    uint16_t s_max_mnt_count;     // Maximal mount count
    uint16_t s_magic;             // Magic signature (0xEF53)
    uint16_t s_state;             // File system state
    uint16_t s_errors;            // Behavior when detecting errors
    uint16_t s_minor_rev_level;   // Minor revision level
    uint32_t s_lastcheck;         // Time of last check
    uint32_t s_checkinterval;     // Max time between checks
    uint32_t s_creator_os;        // OS where created
    uint32_t s_rev_level;         // Revision level
    uint16_t s_def_resuid;        // Default uid for reserved blocks
    uint16_t s_def_resgid;        // Default gid for reserved blocks
    
    // EXT2_DYNAMIC_REV Specific Fields
    uint32_t s_first_ino;         // First non-reserved inode
    uint16_t s_inode_size;        // Size of inode structure
    uint16_t s_block_group_nr;    // Block group # of this superblock
    uint32_t s_feature_compat;    // Compatible feature set
    uint32_t s_feature_incompat;  // Incompatible feature set
    uint32_t s_feature_ro_compat; // Read-only compatible feature set
    uint8_t  s_uuid[16];          // 128-bit filesystem ID
    char     s_volume_name[16];   // Volume name
    char     s_last_mounted[64];  // Directory where last mounted
    uint32_t s_algo_bitmap;       // For compression

    // Performance hints
    uint8_t  s_prealloc_blocks;
    uint8_t  s_prealloc_dir_blocks;
    uint16_t s_padding1;

    // Journaling support (if enabled)
    uint8_t  s_journal_uuid[16];
    uint32_t s_journal_inum;
    uint32_t s_journal_dev;
    uint32_t s_last_orphan;
    uint32_t s_hash_seed[4];
    uint8_t  s_def_hash_version;
    uint8_t  s_padding[3];
    uint32_t s_default_mount_options;
    uint32_t s_first_meta_bg;

    // struct must be 1024 bytes
    uint8_t  unused[788];
} __attribute__((packed));



struct ext2_block_group_descriptor {
    uint32_t bg_block_bitmap;      // Blocks bitmap block
    uint32_t bg_inode_bitmap;      // Inodes bitmap block
    uint32_t bg_inode_table;       // Inode table block
    uint16_t bg_free_blocks_count; // Free blocks count
    uint16_t bg_free_inodes_count; // Free inodes count
    uint16_t bg_used_dirs_count;   // Directories count
    uint16_t bg_pad;               // Padding
    uint8_t  bg_reserved[12];      // Reserved
} __attribute__((packed));

struct ext2_inode {
    uint16_t i_mode;        // File type and permissions
    uint16_t i_uid;         // User ID
    uint32_t i_size;        // Lower 32 bits of file size
    uint32_t i_atime;       // Access time
    uint32_t i_ctime;       // Creation time
    uint32_t i_mtime;       // Modification time
    uint32_t i_dtime;       // Deletion time
    uint16_t i_gid;         // Group ID
    uint16_t i_links_count; // Hard links count
    uint32_t i_blocks;      // Number of 512-byte blocks reserved
    uint32_t i_flags;       // Flags
    uint32_t i_osd1;        // OS specific
    uint32_t i_block[15];   // Pointers to data blocks (0-11 are direct)
    uint32_t i_generation;  // File version
    uint32_t i_file_acl;    // File ACL
    uint32_t i_dir_acl;     // Directory ACL
    uint32_t i_faddr;       // Fragment address
    uint32_t i_osd2[3];     // OS specific
} __attribute__((packed));

struct ext2_directory_entry {
    uint32_t inode;         // Inode number
    uint16_t rec_len;       // Directory entry length
    uint8_t  name_len;      // Name length
    uint8_t  file_type;     // File type
    char     name[255];     // File name
} __attribute__((packed));


struct ext2_bg_descriptor {
    uint32_t bg_block_bitmap;
    uint32_t bg_inode_bitmap;
    uint32_t bg_inode_table;
    uint16_t bg_free_blocks_count;
    uint16_t bg_free_inodes_count;
    uint16_t bg_used_dirs_count;
    uint16_t bg_pad;
    uint32_t bg_reserved[3];
} __attribute__((packed));

struct ext2_dir_entry {
    uint32_t inode;
    uint16_t rec_len;      // How long this specific entry is
    uint8_t  name_len;     // How long the actual filename is
    uint8_t  file_type;
    char     name[];       // The actual filename characters
} __attribute__((packed));

bool ext2_init(uint32_t start_lba, uint16_t port);
uint32_t get_inode_table(uint16_t port);
void ext2_read_block(uint32_t block_num, void* buffer, uint16_t port);
void ext2_write_block(uint32_t block, void* buffer, uint16_t port);
void list_directory(uint32_t block_num, uint16_t port);
void read_root_inode(uint16_t port);
uint32_t ext2_traverse_path(const char* path, uint16_t port, uint32_t* out_parent = nullptr, char* out_name = nullptr);
uint32_t ext2_find_in_dir(uint32_t dir_inode_num, const char* name, uint8_t* out_type, uint16_t port);
bool ext2_cd(const char* path, uint16_t port);
void ext2_ls(const char* path, uint16_t port);
bool ext2_mkdir(const char* path, uint16_t port);
bool ext2_link_dir_entry(uint32_t parent_inode_num, const char* name, uint32_t new_inode, uint8_t type, uint16_t port);
uint32_t ext2_alloc_block(uint16_t port);
uint32_t ext2_alloc_inode(uint16_t port);
void ext2_read_inode(uint32_t inode_num, ext2_inode* buffer, uint16_t port);
void ext2_write_inode(uint32_t inode_num, ext2_inode* inode_data, uint16_t port);
const char* ext2_get_path();