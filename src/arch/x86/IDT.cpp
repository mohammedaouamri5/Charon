// IDT.cpp
#include "IDT.hpp"
#include "CPU.hpp"
#include "std/types.h"

// Fields: bits 0-15 handler low, 16-31 selector, 32-39 zero, 40-47 attributes,
// 48-63 handler high
static void setEntry(u64 &entry, u32 handler, u16 selector, u8 attributes) {
  entry = (u64)(handler & 0xFFFF);
  entry |= (u64)selector << 16;
  entry |= (u64)attributes << 40;
  entry |= (u64)(handler >> 16) << 48;
}

inline u64 idt[256]; // zero-initialised, filled at run time
extern "C" u32 isrStubTable[256];
extern "C" void idtFlush(IDT_ptr *);

#include "PIC.hpp"
#include "VGA.hpp"

IrqHandler IRQ_handlers[16]; // one slot per IRQ 0-15, zero-init: all nullptr

void InstallIRQ(u8 irq, IrqHandler handler) {
  if (irq >= 16)
    return;
  IRQ_handlers[irq] = handler;
  IRQ::Unmask(irq); // PIC.cpp opens the cascade line for slave IRQs itself
}

void IDT_ptr::init() {
  for (int i = 0; i < 256; i++)
    setEntry(idt[i], isrStubTable[i], 0x08, 0x8E); // interrupt gate, ring 0

  this->limit = sizeof(idt) - 1; // 256 * 8 - 1 = 2047
  this->base = (u32)&idt;
  idtFlush(this); // assembly routine in IDT.s
  CPU::Sti();
}

static const char *exceptionNames[32] = {"Divide Error",
                                         "Debug",
                                         "NMI",
                                         "Breakpoint",
                                         "Overflow",
                                         "Bound Range Exceeded",
                                         "Invalid Opcode",
                                         "Device Not Available",
                                         "Double Fault",
                                         "Coprocessor Segment Overrun",
                                         "Invalid TSS",
                                         "Segment Not Present",
                                         "Stack-Segment Fault",
                                         "General Protection Fault",
                                         "Page Fault",
                                         "Reserved",
                                         "x87 Floating-Point",
                                         "Alignment Check",
                                         "Machine Check",
                                         "SIMD Floating-Point",
                                         "Virtualization",
                                         "Control Protection",
                                         "Reserved",
                                         "Reserved",
                                         "Reserved",
                                         "Reserved",
                                         "Reserved",
                                         "Reserved",
                                         "Hypervisor Injection",
                                         "VMM Communication",
                                         "Security Exception",
                                         "Reserved"};

// ---------------------------------------------------------------- panic print
// Tiny direct-to-VGA printer, independent of the Terminal object: an
// exception can happen before, after or inside terminal code, so the panic
// path must not depend on kernel_main's local state.
namespace {

struct PanicPrinter {
  usize row = 0;
  usize col = 0;
  u8 attribute = VGA::make_attribute(VGA::COLOR_WHITE, VGA::COLOR_RED);

  void put(char c) {
    if (c == '\n') {
      row++;
      col = 0;
      return;
    }
    if (col >= VGA::WIDTH)
      newline_row();
    if (row >= VGA::HEIGHT)
      return; // truncation is better than recursing in a fault handler
    VGA::put_char({col, row}, c, attribute);
    col++;
  }

  void put(const char *s) {
    for (; *s; s++)
      put(*s);
  }

  void putHex(u32 value) { // "0x" + 8 fixed lowercase hex digits
    put("0x");
    static const char digits[] = "0123456789abcdef";
    for (int shift = 28; shift >= 0; shift -= 4)
      put(digits[(value >> shift) & 0xF]);
  }

  void newline_row() {
    row++;
    col = 0;
  }
};

} // namespace

// One dispatcher for everything; the vector decides what happens
extern "C" void isrHandler(Registers *r) {
  // ---- CPU exceptions (vectors 0-31): diagnostics, then halt ------------
  if (r->vector < 32) {
    VGA::clear(); // blank the screen so the panic is readable

    PanicPrinter p;
    p.put("[ PANIC ] ");
    p.put(exceptionNames[r->vector & 31]);
    p.put(" (vector ");
    p.putHex(r->vector);
    p.put(")\n\n");
    p.put("  error code: ");
    p.putHex(r->errorCode);
    p.put('\n');
    p.put("  eip:        ");
    p.putHex(r->eip);
    p.put('\n');
    p.put("  cs:         ");
    p.putHex(r->cs);
    p.put('\n');
    p.put("  eflags:     ");
    p.putHex(r->eflags);
    p.put('\n');
    p.put("\n  system halted, interrupts disabled");

    for (;;)
      asm volatile("cli; hlt");
    // Unreachable: iret below is not taken for exceptions
  }

  // ---- Hardware IRQs (vectors 32-47 = IRQ 0-15) --------------------------
  u8 irq = static_cast<u8>(r->vector - 32);

  // A fake IRQ7/IRQ15 runs no handler and takes no normal EOI;
  // IsSpurious() already sends the one EOI a spurious IRQ15 requires.
  if (PIC::IsSpurious(irq))
    return;

  IrqHandler handler = IRQ_handlers[irq];
  if (handler)
    handler(); // handlers do NOT EOI themselves: policy lives here

  // Exactly one EOI per real IRQ, always, so the PIC keeps delivering.
  PIC::SendEOI(irq);
}
