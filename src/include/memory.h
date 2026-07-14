#include <stdint.h>
#include "efi.h"

struct free_page {
    struct free_page* next;
};

struct FramebufferInfo {
    uint64_t BaseAddress;
    uint32_t Width;
    uint32_t Height;
    uint32_t Pitch;
};

struct BootInfo {
    FramebufferInfo fb; 
    EFI_MEMORY_DESCRIPTOR* mem_map;
    uint64_t mem_map_size;
    uint64_t mem_desc_size;
};

void pmm_free(void* ptr) noexcept;
void* pmm_alloc() noexcept;
void init_pmm(uint32_t map_size, uint32_t desc_size);