#pragma once 
#include <stdint.h>

struct gdt_entry_struct {
  uint16_t limit;
  uint16_t base_low;
  uint8_t base_middle;
  uint8_t access;
  uint8_t flags;
  uint8_t base_high;
} __attribute__((packed));

struct gdt_ptr_struct {
  uint16_t limit;
  uint64_t base; 
} __attribute__((packed)); 

extern "C" {
    void initGdt(); 
    void setGdtGate(uint32_t num, uint64_t base, uint32_t limit, uint8_t access, uint8_t gran); 
    void writeTSS(uint32_t num, uint16_t ss0, uint64_t esp0); 
    void gdt_flush(uint64_t ptr);
    void tss_flush();
}
