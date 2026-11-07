// std/io.hpp
#ifndef IO_HPP
#define IO_HPP

#include "std/types.h"

namespace io {


// Send one byte to an I/O port.
inline void outb(u16 port, u8 value) {
  asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

// Read one byte from an I/O port.
inline u8 inb(u16 port) {
  u8 result;
  asm volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
  return result;
}

// Send a 16-bit word to an I/O port.
inline void outw(u16 port, u16 value) {
  asm volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

// Read a 16-bit word from an I/O port.
inline u16 inw(u16 port) {
  u16 result;
  asm volatile("inw %1, %0" : "=a"(result) : "Nd"(port));
  return result;
}

// Send a 32-bit value to an I/O port (used by PCI config space).
inline void outl(u16 port, u32 value) {
  asm volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

// Read a 32-bit value from an I/O port.
inline u32 inl(u16 port) {
  u32 result;
  asm volatile("inl %1, %0" : "=a"(result) : "Nd"(port));
  return result;
}

// Waste a few microseconds by writing to an unused port (0x80).
// Needed between PIC remap writes on old hardware.
inline void wait() { outb(0x80, 0); }

} // namespace io

#endif
