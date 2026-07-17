#include <stdint.h>
#include "efi.h"

extern "C" char _kernel_start[];
extern "C" char _kernel_end[];

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

struct PageTableEntry {
    uint64_t Value;

    void SetAddress(uint64_t addr) { Value |= (addr & 0x0000FFFFFFFFF000); }
    void SetFlag(uint64_t flag, bool enabled) {
        if (enabled) Value |= flag;
        else Value &= ~flag;
    }
};

void map_page(uint64_t virtual_address, uint64_t physical_address);
void pmm_free(void* ptr) noexcept;
void* pmm_alloc() noexcept;
void init_pmm(uint32_t map_size, uint32_t desc_size);
uint64_t get_cr3();
void map_page(uint64_t virtual_address, uint64_t physical_address);
void tlb_flush(uint64_t virtual_address);
void identity_map_range(uint64_t start_address, uint64_t size);
void init_paging();