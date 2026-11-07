/*
================================================================================
 PIC.cpp - 8259 Programmable Interrupt Controller: implementation
================================================================================

 See PIC.hpp for the overview. This file documents the hardware details.

 I/O PORTS
 ---------
     master command  0x20      master data  0x21
     slave  command  0xA0      slave  data  0xA1

 The command port receives initialization words (ICW1) and operation words
 (OCW2/OCW3, such as EOI and register reads). The data port receives the
 remaining initialization words and doubles as the interrupt mask register
 once initialization finishes: bit N set means line N is masked.

 INITIALIZATION SEQUENCE
 -----------------------
 Writing ICW1 with the INIT bit to a command port starts a 4-step sequence.
 The following three writes to the data port are interpreted as ICW2, ICW3
 and ICW4, in that order, so the order of writes below must not change.

     ICW1  0x11   initialization begins, and ICW4 will follow
     ICW2         vector offset (master 0x20, slave 0x28)
     ICW3         cascade wiring:
                    master: bitmask of the line the slave is on  (1 << 2)
                    slave:  its own cascade identity             (2)
     ICW4  0x01   8086/88 mode rather than 8080/85 mode

 After ICW4 the PIC is operational and the data port is the mask register
 again, so the final writes set the initial mask. Everything is masked except
 IRQ2, otherwise no slave interrupt could ever be delivered.

 WHY wait() BETWEEN WRITES
 -------------------------
 Old PICs are slow and may miss back-to-back port writes. io::wait() writes
 to unused port 0x80, which costs roughly a microsecond. Emulators and modern
 hardware don't need it, but it is harmless.

 END OF INTERRUPT (EOI)
 ----------------------
 Writing 0x20 to a command port clears the highest-priority bit in that PIC's
 In-Service Register. For IRQ 8-15 both chips must be told: the slave first,
 then the master, because the master also has IRQ2 in service for the cascade.

 READING IRR / ISR
 -----------------
 Writing an OCW3 value to the command port selects which register the next
 read from that port returns:

     0x0A  Interrupt Request Register  (lines asserted but not yet serviced)
     0x0B  In-Service Register         (lines the CPU is handling now)

 Both chips are selected, then read, and the results are combined into one
 16-bit value (slave << 8 | master).

 SPURIOUS DETECTION
 ------------------
 A genuine IRQ7 or IRQ15 has its bit set in the ISR. If the bit is clear the
 interrupt is fake and the handler must skip the normal EOI.
     spurious IRQ7:   no EOI at all.
     spurious IRQ15:  the master really did receive IRQ2 from the slave, so
                      it still needs an EOI, but the slave does not.

 MASK BIT LOOKUP
 ---------------
 resolve() maps an IRQ number 0-15 to the data port of the chip it lives on
 and its bit position within that chip (line 11 -> slave, bit 3). Mask,
 Unmask and IsMasked all share it.

================================================================================
*/

#include "PIC.hpp"
#include "std/io.hpp"

namespace {

constexpr u16 masterCommand = 0x20;
constexpr u16 masterData = 0x21;
constexpr u16 slaveCommand = 0xA0;
constexpr u16 slaveData = 0xA1;

constexpr u8 icw1Init = 0x10;
constexpr u8 icw1ICW4 = 0x01;
constexpr u8 icw4Mode8086 = 0x01;

constexpr u8 ocw3ReadIRR = 0x0A;
constexpr u8 ocw3ReadISR = 0x0B;

constexpr u8 eoi = 0x20;
constexpr u8 cascadeIRQ = 2;
constexpr u8 maskAllLines = 0xFF;

struct Line {
  u16 port;
  u8 bit;
};

Line resolve(u8 line) {
  if (line < 8)
    return {masterData, line};
  return {slaveData, static_cast<u8>(line - 8)};
}

u16 readRegister(u8 ocw3) {
  io::outb(masterCommand, ocw3);
  io::outb(slaveCommand, ocw3);
  u8 low = io::inb(masterCommand);
  u8 high = io::inb(slaveCommand);
  return static_cast<u16>((high << 8) | low);
}

} // namespace

namespace PIC {

void Remap(u8 masterOffset, u8 slaveOffset) {
  using namespace io;

  outb(masterCommand, icw1Init | icw1ICW4);
  wait();
  outb(slaveCommand, icw1Init | icw1ICW4);
  wait();

  outb(masterData, masterOffset);
  wait();
  outb(slaveData, slaveOffset);
  wait();

  outb(masterData, 1 << cascadeIRQ);
  wait();
  outb(slaveData, cascadeIRQ);
  wait();

  outb(masterData, icw4Mode8086);
  wait();
  outb(slaveData, icw4Mode8086);
  wait();

  outb(masterData, static_cast<u8>(~(1 << cascadeIRQ)));
  outb(slaveData, maskAllLines);
}

void MaskAll() {
  io::outb(masterData, maskAllLines);
  io::outb(slaveData, maskAllLines);
}

void SendEOI(u8 irq) {
  if (irq >= 8)
    io::outb(slaveCommand, eoi);
  io::outb(masterCommand, eoi);
}

u16 IRR() { return readRegister(ocw3ReadIRR); }

u16 ISR() { return readRegister(ocw3ReadISR); }

bool IsSpurious(u8 irq) {
  if (irq != 7 && irq != 15)
    return false;

  if (ISR() & (1u << irq))
    return false;

  if (irq == 15)
    io::outb(masterCommand, eoi);
  return true;
}

} // namespace PIC

namespace IRQ {

void Mask(u8 line) {
  if (line > 15)
    return;
  Line loc = resolve(line);
  u8 value = io::inb(loc.port) | (1 << loc.bit);
  io::outb(loc.port, value);
}

void Unmask(u8 line) {
  if (line > 15)
    return;
  if (line >= 8)
    Unmask(cascadeIRQ);
  Line loc = resolve(line);
  u8 value = io::inb(loc.port) & ~(1 << loc.bit);
  io::outb(loc.port, value);
}

bool IsMasked(u8 line) {
  if (line > 15)
    return true;
  Line loc = resolve(line);
  return io::inb(loc.port) & (1 << loc.bit);
}

} // namespace IRQ
/*

So here is what happens, step by step, for the closest real case: IRQ12 (mouse)
and IRQ1 (keyboard) fire at the same moment.

- The mouse raises IRQ12 on the slave, and the slave sets bit 4 in its own
Interrupt Request Register (IRR).
- The keyboard raises IRQ1 on the master, and the master sets bit 1 in its IRR.
- The slave sees a pending, unmasked request and raises its output wire, which
is the master's IRQ2 input. The master sets bit 2 in its IRR.
- The master now has two pending bits: IRQ1 and IRQ2 (the slave).
- There is no queue and no overwriting. The IRR is just a row of bits, one per
line, and every pending request keeps its own bit.
- The master's priority resolver picks the winner. Lower number means higher
priority, so IRQ1 beats IRQ2.
- The master raises the INT pin to the CPU for IRQ1.
- The CPU finishes its current instruction, sees INT (if interrupts are enabled
with `sti`), and sends the acknowledge signal back.
- The master moves IRQ1 from the IRR to the In-Service Register (ISR) and sends
the vector 0x21.
- The CPU jumps to your keyboard handler through the IDT.
- IRQ2 is still pending in the master's IRR the whole time. The slave keeps its
wire raised, and the mouse bit stays set in the slave's IRR.
- While IRQ1 is in service, the master only lets through requests of higher
priority than IRQ1. IRQ2 is lower, so it waits.
- Your keyboard handler sends the EOI. The master clears IRQ1 from the ISR.
- Nothing higher is in service now, so the master raises INT again for IRQ2.
- The CPU acknowledges. The master sees the request came from the cascade line,
so it passes the acknowledge to the slave.
- The slave moves IRQ12 from its IRR to its ISR and puts vector 0x2C on the bus.
The master's ISR also marks IRQ2 as in service.
- The CPU runs your mouse handler.
- The handler sends the EOI to the slave first, then to the master. This clears
IRQ12 in the slave's ISR and IRQ2 in the master's ISR.
- Both chips are idle again.

Some other things that matter here:

- **Priority order:** the master's order is IRQ0, IRQ1, then the whole slave
group (8 to 15), then IRQ3 to IRQ7. The slave sits at IRQ2's position, so all of
IRQ8-15 outrank IRQ3-7 and lose only to the timer and keyboard. Inside the
slave, 8 beats 9, which beats 10, and so on.
- **If the CPU can't take interrupts yet** (inside a handler without `sti`, or
after `cli`), nothing is lost. The bits wait in the IRR until the CPU is ready.
- **If one line fires twice before being served**, the two events merge into
one. The IRR has one bit per line, not a counter, so the second pulse is lost.
This is why handlers should drain the device (read everything it has) each time
they run.
- **If a higher-priority IRQ arrives during a handler,** it can interrupt the
lower one, but only once you've re-enabled interrupts with `sti` inside the
handler. By default, the CPU clears the interrupt flag on entry through an
interrupt gate, so nesting doesn't happen unless you allow it.
- **Masked lines** never reach the priority resolver at all. A masked IRQ2 on
the master blocks every slave line, which is why `ClearMask` opens it for you.
 */
