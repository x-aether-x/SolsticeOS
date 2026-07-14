#include <stdint.h>
#include "memory.h"
#include <stddef.h>
#include "efi.h"

#define MEM_MAP 0x80000 

free_page* free_list_head = nullptr;
size_t total_free_pages = 0;

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
