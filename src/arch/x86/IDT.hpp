// IDT.hpp
#ifndef IDT_HPP
#define IDT_HPP
#include "std/types.h"

struct __attribute__((packed)) IDT_ptr {
  u16 limit;
  u32 base;
  void init(); // assembly routine in IDT.s
};

struct Registers {
  u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;  // pusha order (esp is a dummy value)
  u32 vector;     // pushed by the stub
  u32 errorCode;  // pushed by CPU, or 0 from the stub
  u32 eip, cs, eflags;
};

extern "C" void isrHandler(Registers *r);
using IrqHandler = void (*)();

// One slot per IRQ 0..15, zero-init; nullptr means no handler installed yet.
extern IrqHandler IRQ_handlers[16];

// Register `handler` for IRQ line `irq` (and unmask that line).
// Silently rejects irq >= 16, same early-return style as PIC.cpp IRQ::Mask.
// Real handlers must NOT call PIC::SendEOI() themselves; the common path does.
void InstallIRQ(u8 irq, IrqHandler handler);

#endif
