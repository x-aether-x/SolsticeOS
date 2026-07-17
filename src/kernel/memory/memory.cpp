#include <stdint.h>
#include "memory.h"
#include <stddef.h>
#include "efi.h"

#define MEM_MAP 0x80000 

free_page* free_list_head = nullptr;
size_t total_free_pages = 0;

uint64_t kernel_size = (uint64_t)_kernel_end - (uint64_t)_kernel_start;

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

#define MEM_MAP 0x80000ULL 

void init_pmm(uint32_t map_size, uint32_t desc_size) {
    for (uint32_t i = 0; i < (map_size / desc_size); i++){
        EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)(MEM_MAP + (i * desc_size));
        
        if (desc->Type == EfiConventionalMemory) {
            for (uint64_t j = 0; j < desc->NumberOfPages; j++) { 
                uint64_t page_address = desc->PhysicalStart + (j * 4096);
                if (page_address >= 0x80000 && page_address < 0x200000) {
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

void map_page(uint64_t virtual_address, uint64_t physical_address) {
    uint64_t cr3 = get_cr3();
    uint64_t pml4_addr = cr3 & 0x0000FFFFFFFFF000;
    uint64_t* pml4 = (uint64_t*)pml4_addr;
    uint64_t pml4_index = (virtual_address >> 39) & 0x1FF;

    uint64_t entry = pml4[pml4_index];
    if (!(entry & 1)) {
        void* new_frame = pmm_alloc();
        uint64_t* new_table = (uint64_t*)new_frame;
        for (int i = 0; i < 512; i++) {
            new_table[i] = 0;
        }
        pml4[pml4_index] = (uint64_t)new_frame | 0b11;
    }

    entry = pml4[pml4_index]; 
    uint64_t next_table_phys_addr = entry & 0x0000FFFFFFFFF000;
    uint64_t* pdpt = (uint64_t*)next_table_phys_addr;
    uint64_t pdpt_index = (virtual_address >> 30) & 0x1FF;

    uint64_t pdpt_entry = pdpt[pdpt_index];
    if (!(pdpt_entry & 1)) {
        void* new_frame = pmm_alloc();
        uint64_t* new_table = (uint64_t*)new_frame;
        for (int i = 0; i < 512; i++) {
            new_table[i] = 0;
        }
        pdpt[pdpt_index] = (uint64_t)new_frame | 0b11;
    }

    pdpt_entry = pdpt[pdpt_index];
    uint64_t pd_phys_addr = pdpt_entry & 0x0000FFFFFFFFF000;
    uint64_t* pd = (uint64_t*)pd_phys_addr;
    uint64_t pd_index = (virtual_address >> 21) & 0x1FF;

    uint64_t pd_entry = pd[pd_index];
    if (!(pd_entry & 1)) {
        void* new_frame = pmm_alloc();
        uint64_t* new_table = (uint64_t*)new_frame;
        for (int i = 0; i < 512; i++) {
            new_table[i] = 0;
        }
        pd[pd_index] = (uint64_t)new_frame | 0b11;
    }

    pd_entry = pd[pd_index];
    uint64_t pt_phys_addr = pd_entry & 0x0000FFFFFFFFF000;
    uint64_t* pt = (uint64_t*)pt_phys_addr;
    uint64_t pt_index = (virtual_address >> 12) & 0x1FF;

    pt[pt_index] = (physical_address & 0x0000FFFFFFFFF000) | 0b11;

    tlb_flush(virtual_address);
}

void tlb_flush(uint64_t virtual_address) {
    asm volatile("invlpg (%0)" : : "r"(virtual_address) : "memory");
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
    identity_map_range((uint64_t)_kernel_start, kernel_size); // map the kernel
    identity_map_range(fb->BaseAddress, fb->Height * fb->Pitch); // map the framebuffer
    identity_map_range(0x9000, 4096);
}