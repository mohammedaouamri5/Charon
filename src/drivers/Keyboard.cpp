/*
================================================================================
 Keyboard.cpp - PS/2 set-2 keyboard driver: IRQ1, event queue, key state
================================================================================

 PORTS
 -----
     0x60  keyboard data register (read one scancode per IRQ1)
     0x64  controller status (not used: the IRQ already tells us data is ready)

 SCANCODING (set-2)
 ------------------
 A press sends one byte; a release sends F0 first, then the same byte.
 Extended keys (arrows, right modifiers, ...) come as a 0xE0 escape byte
 first, then the real code (so a release is E0 F0 code). The escape and
 break bytes arrive as their own IRQ1, so the handler latches them and
 waits for the next byte. Nesting is impossible: IF is cleared while the
 handler runs. Unlike set-1, set-2 codes use all 8 bits: there is no
 bit-7 press/release trick, so the state table is [2][256].

 DESIGN
 ------
 The IRQ1 handler only records state: it updates the down[ext][code] table
 (read directly by IsDown) and pushes a KeyEvent onto the ring queue (read
 by Poll). Consumers are decoupled from the interrupt: no printing, no EOI
 and no queue locking in the handler. The dispatcher in IDT.cpp sends the
 one EOI every real IRQ needs, so handlers must not do it themselves.

 The queue consumer takes the guard: keyboard's user calls IRQ::Poll
 without ever seeing Lock, so nobody can forget to disable interrupts
 around a pop.

 NOTES
 -----
 - No controller translation: the controller must feed raw set-2 bytes.
 - Pause key: its 8-byte E1 burst is not swallowed; stray bytes may slip
   through as spurious events.
 - Ring capacity: if nobody polls long enough, Push overwrites the oldest
   events rather than dropping new ones.

 GO-STYLE: exported names PascalCase, file-private camelCase.
================================================================================
*/

#include "drivers/Keyboard.hpp"
#include "arch/x86/IDT.hpp"
#include "drivers/PS2.hpp"
#include "std/struct/ring.hpp"
#include "std/types.h"

namespace {


SPSCRing<IRQ::KeyEvent, 64> events;
volatile bool down[2][256];

void onIRQ1() {

  static bool ext = false; 
  static bool rel = false; 
  u8 data;
  u8 status;
  if (PS2::Pull(data, status) != PS2::Err::OK) {
    return; // empty buffer or a mouse byte: nothing keyboard-ish to do
  }
  if (data == 0xE0) { ext = true; return; }
  if (data == 0xF0) { rel = true; return; }

  if (data == 0xFA || data == 0xFE || data == 0xAA ||
      data == 0xFC || data == 0x00 || data == 0xFF) {
    return; 
  }
  bool pressed = !rel;
  down[ext][data] = pressed;
  events.Push({data, ext, pressed});
  ext = false;
  rel = false;
}

} // namespace

namespace IRQ {

void Init() {
  InstallIRQ(1, onIRQ1);
}

bool Poll(KeyEvent &out) { return events.Pop(out); }

bool IsDown(u8 code, bool ext) {
  return down[ext][code]; // one byte read: atomic
}

} // namespace IRQ
