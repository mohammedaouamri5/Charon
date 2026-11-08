// std/struct/irqsave.hpp
#ifndef STD_STRUCT_IRQSAVE_HPP
#define STD_STRUCT_IRQSAVE_HPP

#include "std/types.h"

namespace IRQ {

// RAII guard: reads EFLAGS and disables interrupts on construction; on
// destruction re-enables them only if IF was set when the guard was taken.
// `irq` is the line the caller is protecting (documentation value only).
struct Lock {
  u32 flags;

  Lock([[maybe_unused]] u8 irq) {
    asm volatile("pushf; pop %0; cli" : "=r"(flags) :: "memory");
  }
  ~Lock() {
    if (flags & 0x200)
      asm volatile("sti" ::: "memory");
  }

  Lock(const Lock &) = delete;
  Lock &operator=(const Lock &) = delete;
};

} // namespace IRQ

#endif
