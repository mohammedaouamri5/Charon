

#ifndef __CPU_HPP__
#define __CPU_HPP__

namespace CPU {
inline void Cli() { asm volatile("cli" : : : "memory"); }
inline void Sti() { asm volatile("sti" : : : "memory"); }
} // namespace CPU

#endif
