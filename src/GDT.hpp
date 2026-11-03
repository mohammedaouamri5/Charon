#ifndef GDT_HPP
#define GDT_HPP


#include "std/types.h"
struct GTD_ptr;

#ifdef __cplusplus
extern "C" {
#endif

/* Implemented in GDT.s */
void gdt_flush(struct GTD_ptr *gdtr);

#ifdef __cplusplus
}
#endif

struct __attribute__((packed)) GTD_ptr {
  u16 limit; // size of the table in bytes - 1
  u32 base;  // address of the table
  void init(void);
};


#endif
