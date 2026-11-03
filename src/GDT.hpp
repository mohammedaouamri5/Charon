#ifndef GDT_HPP
#define GDT_HPP


#include "std/types.h"
struct GDT_ptr;

#ifdef __cplusplus
extern "C" {
#endif

/* Implemented in GDT.s */
void gdt_flush(struct GDT_ptr *gdtr);

#ifdef __cplusplus
}
#endif

struct __attribute__((packed)) GDT_ptr {
  u16 limit; // size of the table in bytes - 1
  u32 base;  // address of the table
  void init(void);
};


#endif
