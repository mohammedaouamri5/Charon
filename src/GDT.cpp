



#include "GDT.hpp"



inline uint64_t gdt[5] = {
    0x0000000000000000, // 0x00 null descriptor
    0x00CF9A000000FFFF, // 0x08 kernel code
    0x00CF92000000FFFF, // 0x10 kernel data
    0x00CFFA000000FFFF, // 0x18 user code
    0x00CFF2000000FFFF  // 0x20 user data
};


void GTD_ptr::init() {

  this->limit = sizeof(gdt) - 1; // 5 * 8 - 1 = 39
  this->base = (uint32_t)&gdt;
  gdt_flush(this); // assembly routine in GDT.s
}

inline void gdt_init(void) {
}
