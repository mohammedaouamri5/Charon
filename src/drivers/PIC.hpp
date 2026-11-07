// PIC.hpp
/*
================================================================================
 PIC.hpp - 8259 Programmable Interrupt Controller: public interface
================================================================================

 WHAT THE PIC IS
 ---------------
 Two chained Intel 8259 chips that sit between hardware devices and the CPU.
 Devices raise one of 16 IRQ lines. The PIC decides whether the request is
 allowed through, then hands the CPU a vector number so it can find the right
 entry in the IDT.

     master PIC (IRQ 0-7)  --->  CPU INTR pin
         ^
         | IRQ2 (cascade)
         |
     slave PIC (IRQ 8-15)

 The slave does not talk to the CPU. It signals the master on IRQ2, and the
 master forwards it. This is why unmasking any line from 8 to 15 also
 requires IRQ2 to be unmasked.

 WHY A REMAP IS NEEDED
 ---------------------
 At power-up the master delivers IRQ0-7 as vectors 8-15. The CPU already
 reserves vectors 0-31 for its own exceptions, so a timer tick (IRQ0 -> vector
8) would look exactly like a double fault. Remap() moves the lines to vectors
32-47:

     IRQ 0-7   ->  vectors 32-39   (MasterOffset = 0x20)
     IRQ 8-15  ->  vectors 40-47   (SlaveOffset  = 0x28)

 STATE AFTER Remap()
 -------------------
 Every line is masked except the cascade (IRQ2). Nothing can interrupt the
 CPU until you call irq::Unmask() for a line, so enable a line only after
 its handler is installed in the IDT.

 THE LIFE OF AN INTERRUPT
 ------------------------
 1. A device raises its IRQ line.
 2. The PIC checks the mask and priority (lower IRQ number wins).
 3. The PIC signals the CPU, and the CPU reads the vector number.
 4. The CPU saves its state and jumps to the IDT handler for that vector.
 5. The handler does its work, then calls SendEOI() so the PIC knows it may
    deliver further interrupts on that line (and lower-priority ones).
 6. The CPU resumes the interrupted code.

 If SendEOI() is forgotten, that line never fires again.

 SPURIOUS INTERRUPTS
 -------------------
 The PIC may report IRQ7 (master) or IRQ15 (slave) when no device actually
 requested service, for example when a line glitches and drops before the CPU
 acknowledges it. Call IsSpurious() first in the handler for those two lines.
 If it returns true, return immediately WITHOUT calling SendEOI(). IsSpurious()
 already sends the single EOI that a spurious IRQ15 needs.

 API OVERVIEW
 ------------
 namespace pic
   MasterOffset, SlaveOffset   standard vector offsets (0x20, 0x28)
   Remap(master, slave)        reprogram both PICs, mask everything but IRQ2
   MaskAll()                   silence all 16 lines
   SendEOI(irq)                acknowledge an IRQ (0-15); call at handler end
   IRR()                       Interrupt Request Register: lines asserted
   ISR()                       In-Service Register: lines being handled
   IsSpurious(irq)             detect fake IRQ7 / IRQ15

 namespace irq
   Mask(line)                  disable one line (0-15)
   Unmask(line)                enable one line; lines 8-15 also open IRQ2
   IsMasked(line)              query a line's mask bit

 Both registers (IRR, ISR) are returned as one 16-bit value:
 slave in the high byte, master in the low byte.

 BOOT ORDER
 ----------
     gdt::Init();
     idt::Init();                 vectors 32-47 must have working stubs
     pic::Remap(pic::MasterOffset, pic::SlaveOffset);
     irq::Unmask(1);              once the keyboard handler exists
     sti

 A typical IRQ handler, given irq = vector - 32:

     if (pic::IsSpurious(irq)) return;
     ... service the device ...
     pic::SendEOI(irq);

 NOTE ON LATER HARDWARE
 ----------------------
 The PIC is the legacy design. Multi-core and x86_64 systems use the APIC,
 and pic::MaskAll() is what you call before switching to it.
================================================================================
*/

#pragma once

#include "std/types.h"

namespace PIC {

constexpr u8 MasterOffset = 0x20;
constexpr u8 SlaveOffset = 0x28;

void Remap(u8 masterOffset = PIC::MasterOffset,
           u8 slaveOffset = PIC::SlaveOffset);
void MaskAll();
void SendEOI(u8 irq);
u16 IRR();
u16 ISR();
bool IsSpurious(u8 irq);

} // namespace PIC

namespace IRQ {

void Mask(u8 line);
void Unmask(u8 line);
bool IsMasked(u8 line);

} // namespace IRQ
