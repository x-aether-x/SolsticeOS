#pragma once
#include <stdint.h>
#include <stddef.h>

extern "C" char _kernel_start[];
extern "C" char _kernel_end[];

struct MemMapDescriptor {
    uint32_t Type;
    uint32_t Padding;
    uint64_t PhysicalStart;
    uint64_t VirtualStart;
    uint64_t NumberOfPages;
    uint64_t Attribute;
};

#define MEM_TYPE_CONVENTIONAL 7

struct free_page {
    struct free_page* next;
};

struct FramebufferInfo {
    uint64_t BaseAddress;
    uint32_t Width;
    uint32_t Height;
    uint32_t Pitch;
    uint64_t MapSize;   // memory map info passed by the bootloader
    uint64_t DescSize;
};

struct KmemCache;

struct Slab {
    uint64_t bitmap;
    Slab* next;
    uint32_t free_slots;
    KmemCache* cache;
};

struct KmemCache {
    uint32_t object_size;
    Slab* first_slab;
};

struct PageTableEntry {
    uint64_t Value;

    void SetAddress(uint64_t addr) { Value |= (addr & 0x0000FFFFFFFFF000); }
    void SetFlag(uint64_t flag, bool enabled) {
        if (enabled) Value |= flag;
        else Value &= ~flag;
    }
};

void pmm_free(void* ptr) noexcept;
void* pmm_alloc() noexcept;
void init_pmm(uint32_t map_size, uint32_t desc_size);
extern size_t total_free_pages;

uint64_t get_cr3();
void map_page(uint64_t virtual_address, uint64_t physical_address);
void tlb_flush(uint64_t virtual_address);
void identity_map_range(uint64_t start_address, uint64_t size);
void init_paging();

void init_kmalloc();
void* kmalloc(uint64_t target);
void kfree(void* ptr);

void* pmm_alloc_pages(uint64_t count);