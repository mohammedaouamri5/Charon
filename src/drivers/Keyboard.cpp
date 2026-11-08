/*
================================================================================
 Keyboard.cpp - PS/2 set-1 keyboard driver: IRQ1, event queue, key state
================================================================================

 PORTS
 -----
     0x60  keyboard data register (read one scancode per IRQ1)
     0x64  controller status (not used: the IRQ already tells us data is ready)

 SCANCODING (set-1)
 ------------------
 Every key press ("make") sends one byte; its release ("break") sends the
 same byte with bit 7 set. Extended keys (arrows, right modifiers, ...) come
 as a 0xE0 escape byte first, then the real scancode. The escape byte
 arrives as its own IRQ1, so the handler latches it in `ext`, waits for the
 next byte, then clears the latch. Nesting is impossible: IF is cleared
 while the handler runs.

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
 - No controller translation / set-2 handling: works with QEMU defaults.
 - Ring capacity: if nobody polls long enough, Push overwrites the oldest
   events rather than dropping new ones.

 GO-STYLE: exported names PascalCase, file-private camelCase.
================================================================================
*/

#include "Keyboard.hpp"
#include "IDT.hpp"
#include "std/struct/ring.hpp"
#include "std/struct/irqsave.hpp"
#include "std/io.hpp"

namespace {

constexpr u16 dataPort = 0x60;

Ring<IRQ::KeyEvent, 64> events;
volatile bool down[2][128];

// IRQ1 handler: no guard needed, IF is already cleared on entry.
void onIRQ1() {
  static bool ext = false;
  u8 sc = io::inb(dataPort);
  if (sc == 0xE0) {
    ext = true; // escape prefix: the next byte carries the real code
    return;
  }
  bool pressed = !(sc & 0x80);
  u8 code = sc & 0x7F;
  down[ext][code] = pressed;
  events.Push({code, ext, pressed});
  ext = false;
}

} // namespace

namespace IRQ {

void Init() {
  // InstallIRQ also unmasks the line (see IDT.cpp), like PIT::Init does.
  InstallIRQ(1, onIRQ1);
}

bool Poll(KeyEvent &out) {
  Lock g(1); // the guard lives here so no caller can forget it
  return events.PopOld(out);
}

bool IsDown(u8 code, bool ext) {
  return down[ext][code]; // one byte read: atomic
}

} // namespace IRQ
