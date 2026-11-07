/*
================================================================================
 PIT.cpp - 8253/8254 Programmable Interval Timer: implementation
================================================================================

 See PIT.hpp for the overview.

 PORTS
 -----
     0x43  mode/command register (write only)
     0x40  channel 0 data

 DIVISOR
 -------
 divisor = 1193182 / 100 = 11931 (integer division), so the real rate is
 1193182 / 11931 = 100.0007 Hz. The error is about 7 parts per million, which
 is roughly 0.6 seconds per day and is irrelevant here.

 COUNTING
 --------
 interrupts counts raw 10 ms interrupts. When it reaches 10, it resets and
 ticks is incremented, giving one tick per 100 ms. Both variables are volatile
 because the interrupt handler writes them while the main code reads them.
 Ticks() reads a single 32-bit aligned value, which the CPU does atomically.

 NAMING
 ------
 Go-style: exported names are PascalCase, file-private names are camelCase.
================================================================================
*/

#include "PIT.hpp"
#include "IDT.hpp"
#include "PIC.hpp"
#include "std/io.hpp"

namespace {

constexpr u16 commandPort = 0x43;
constexpr u16 channel0Port = 0x40;
constexpr u8 channel0Mode2Binary = 0x34;
constexpr u32 inputClock = 1193182;
constexpr u32 interruptsPerTick = 10;

volatile u32 ticks = 0;
volatile u32 interrupts = 0;

void onIRQ0() {
  interrupts = interrupts + 1;
  if (interrupts >= interruptsPerTick) {
    interrupts = 0;
    ticks = ticks + 1;
  }
}

} // namespace

namespace PIT {

void Init() {
  u16 divisor = static_cast<u16>(inputClock / Frequency);

  io::outb(commandPort, channel0Mode2Binary);
  io::outb(channel0Port, static_cast<u8>(divisor & 0xFF));
  io::outb(channel0Port, static_cast<u8>(divisor >> 8));

    InstallIRQ(0, onIRQ0);
}

u32 Ticks() { return ticks; }

} // namespace PIT
