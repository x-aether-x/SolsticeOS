#include <stdint.h>
#include "memory.h"
#include <stddef.h>

#define MEM_MAP 0x80000ULL
#define NUM_CACHES 9

KmemCache kmem_caches[NUM_CACHES];

free_page* free_list_head = nullptr;
size_t total_free_pages = 0;

// ------------------- PMM -----------------

// pop
void* pmm_alloc() noexcept {
    if (free_list_head == nullptr) return nullptr;

    void* allocated_frame = free_list_head;
    free_list_head = free_list_head->next;
    total_free_pages--;
    return allocated_frame;
}

// push
void pmm_free(void* ptr) noexcept {
    free_page* frame = static_cast<free_page*>(ptr);
    frame->next = free_list_head;
    free_list_head = frame;
    total_free_pages++;
}

void init_pmm(uint32_t map_size, uint32_t desc_size) {
    if (desc_size == 0) return;

    for (uint32_t i = 0; i < (map_size / desc_size); i++) {
        MemMapDescriptor* desc = (MemMapDescriptor*)(MEM_MAP + (i * desc_size));

        if (desc->Type == MEM_TYPE_CONVENTIONAL) {
            for (uint64_t j = 0; j < desc->NumberOfPages; j++) {
                uint64_t page_address = desc->PhysicalStart + (j * 4096);

                // reserve everything below 2MB: BDA/IVT, kernel stack (0x70000),
                // boot info (0x9000), the memory map copy (0x80000), shell buffers,
                // and the kernel itself at 0x100000
                if (page_address < 0x200000) {
                    continue;
                }

                pmm_free((void*)page_address);
            }
        }
    }
}

// -------------- VMM AND PAGING -----------------

uint64_t get_cr3() {
    uint64_t cr3;
    asm volatile ("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

void tlb_flush(uint64_t virtual_address) {
    asm volatile("invlpg (%0)" : : "r"(virtual_address) : "memory");
}

static uint64_t* get_or_create_table(uint64_t* table, uint64_t index) {
    uint64_t entry = table[index];
    if (!(entry & 1)) {
        void* new_frame = pmm_alloc();
        if (new_frame == nullptr) return nullptr; // out of physical memory
        uint64_t* new_table = (uint64_t*)new_frame;
        for (int i = 0; i < 512; i++) {
            new_table[i] = 0;
        }
        table[index] = (uint64_t)new_frame | 0b11;
        entry = table[index];
    }
    return (uint64_t*)(entry & 0x0000FFFFFFFFF000);
}

#define PAGE_HUGE (1ULL << 7) // PS bit: entry maps a 1GB/2MB page, NOT a table

void map_page(uint64_t virtual_address, uint64_t physical_address) {
    uint64_t* pml4 = (uint64_t*)(get_cr3() & 0x0000FFFFFFFFF000);

    uint64_t* pdpt = get_or_create_table(pml4, (virtual_address >> 39) & 0x1FF);
    if (!pdpt) return;

    // UEFI's page tables use huge pages. If this address is already covered
    // by a 1GB or 2MB page, it's mapped - treating the entry as a table
    // pointer would scribble PTEs into the mapped data (e.g. the framebuffer)
    uint64_t pdpt_entry = pdpt[(virtual_address >> 30) & 0x1FF];
    if ((pdpt_entry & 1) && (pdpt_entry & PAGE_HUGE)) return; // 1GB page

    uint64_t* pd = get_or_create_table(pdpt, (virtual_address >> 30) & 0x1FF);
    if (!pd) return;

    uint64_t pd_entry = pd[(virtual_address >> 21) & 0x1FF];
    if ((pd_entry & 1) && (pd_entry & PAGE_HUGE)) return; // 2MB page

    uint64_t* pt = get_or_create_table(pd, (virtual_address >> 21) & 0x1FF);
    if (!pt) return;

    pt[(virtual_address >> 12) & 0x1FF] = (physical_address & 0x0000FFFFFFFFF000) | 0b11;

    tlb_flush(virtual_address);
}

void identity_map_range(uint64_t start_address, uint64_t size) {
    uint64_t start = start_address & 0x0000FFFFFFFFF000;
    uint64_t end = (start_address + size + 4095) & 0x0000FFFFFFFFF000;

    for (uint64_t addr = start; addr < end; addr += 4096) {
        map_page(addr, addr);
    }
}

void init_paging() {
    FramebufferInfo* fb = (FramebufferInfo*)0x9000;

    // computed here at runtime: global initializers never run in this kernel
    uint64_t kernel_size = (uint64_t)_kernel_end - (uint64_t)_kernel_start;

    identity_map_range((uint64_t)_kernel_start, kernel_size); // map the kernel
    identity_map_range(fb->BaseAddress, (uint64_t)fb->Height * fb->Pitch); // map the framebuffer
    identity_map_range(0x9000, 4096);
    identity_map_range(0x1000000, 0x2000000);
}

void* pmm_alloc_pages(uint64_t count) {
    if (count == 0) return nullptr;
    void* first = pmm_alloc();
    if (first == nullptr) return nullptr;
    for (uint64_t i = 1; i < count; i++) {
        if (pmm_alloc() == nullptr) {
            return nullptr;
        }
    }
    return first;
}

// -------------- SLAB ALLOCATOR ------------------

void init_kmalloc() {
    const uint32_t sizes[NUM_CACHES] = { 32, 64, 128, 256, 512, 1024, 2048, 5096, 10192};
    for (int i = 0; i < NUM_CACHES; i++) {
        kmem_caches[i].object_size = sizes[i];
        kmem_caches[i].first_slab = nullptr;
    }
}

void* kmalloc(uint64_t target) {
    if (target == 0) return nullptr;

    for (int i = 0; i < NUM_CACHES; i++) {
        if (kmem_caches[i].object_size >= target) {
            KmemCache& cache = kmem_caches[i];
            Slab* current_slab = cache.first_slab;

            while (current_slab != nullptr && current_slab->free_slots == 0) { // traverse linked list
                current_slab = current_slab->next;
            }

            if (current_slab == nullptr) {
                current_slab = (Slab*)pmm_alloc();
                if (current_slab == nullptr) return nullptr; // out of physical memory
                current_slab->bitmap = 0;
                current_slab->free_slots = (4096 - sizeof(Slab)) / cache.object_size;
                current_slab->next = cache.first_slab;
                current_slab->cache = &cache;
                cache.first_slab = current_slab;
            }

            int slot_index = __builtin_ctzll(~current_slab->bitmap);
            current_slab->free_slots--;

            current_slab->bitmap |= (1ULL << slot_index);
            void* slot_addr = (uint8_t*)(current_slab + 1) + (slot_index * cache.object_size);

            return slot_addr;
        }
    }
    return nullptr; // larger than the biggest cache (1024)
}

void kfree(void* ptr) {
    if (ptr == nullptr) return;
    Slab* slab_header = (Slab*)((uint64_t)ptr & ~0xFFFULL);
    uint64_t byte_offset = (uint8_t*)ptr - (uint8_t*)(slab_header + 1);
    uint64_t slot_index = byte_offset / slab_header->cache->object_size;

    slab_header->bitmap &= ~(1ULL << slot_index);
    slab_header->free_slots++;
}