/*
================================================================================
 PIT.hpp - 8253/8254 Programmable Interval Timer: public interface
================================================================================

 WHAT IT DOES
 ------------
 Configures PIT channel 0 as a rate generator so it raises IRQ0 at a steady
 rate. A handler counts those interrupts and exposes a tick counter that
 advances once every 100 ms.

 WHY NOT ONE INTERRUPT EVERY 100 MS
 ----------------------------------
 The PIT input clock is 1193182 Hz and the divisor is 16 bits (1..65536, where
 0 means 65536). The slowest possible rate is 1193182 / 65536 = 18.2 Hz, which
 is about 55 ms. A 100 ms period (10 Hz) needs a divisor of 119318, which does
 not fit. So the PIT runs at 100 Hz (divisor 11931, an interrupt every 10 ms)
 and every 10th interrupt advances the 100 ms tick.

 CONFIGURATION
 -------------
 One command byte is written to port 0x43, then the divisor is written to
 channel 0's data port 0x40, low byte first, then high byte.

     command 0x34 = 00 11 010 0
                    |  |  |   +-- binary counting
                    |  |  +------ mode 2, rate generator
                    |  +--------- access: low byte then high byte
                    +------------ channel 0

 In mode 2 the output stays high and drops low for one input clock each time
 the counter reaches 1, then the counter reloads by itself. That falling edge
 is the interrupt, so it fires forever with no further programming.

 INTERRUPT BEHAVIOUR
 -------------------
 The handler runs through InstallIRQ(0, ...), which also unmasks IRQ0. The
 dispatcher sends the EOI, so the handler must not. While the handler runs the
 CPU's interrupt flag is clear (interrupt gate), so IRQ0 cannot nest. A request
 that arrives meanwhile is held by the PIC and delivered after the EOI. If the
 handler ever took longer than 10 ms, two requests would merge into one and
 time would drift, so keep it tiny.

 API
 ---
 pit::Init()      program channel 0 and install the IRQ0 handler
 pit::Ticks()     number of 100 ms ticks since Init()
 pit::Frequency   interrupt rate in Hz (100)

 BOOT ORDER
 ----------
     gdt init, idt init, PIC remap, pit::Init(), then sti.
 Interrupts stay disabled until sti, so nothing fires before that point.
================================================================================
*/

#ifndef __PIT_HPP__
#define __PIT_HPP__

#include "std/types.h"

namespace PIT {

constexpr u32 Frequency = 100;

void Init();
u32 Ticks();

} // namespace PIT
#endif
