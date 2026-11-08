#include "GDT.hpp"
#include "Keyboard.hpp"
#include "IDT.hpp"
#include "PIC.hpp"
#include "PIT.hpp"
#include "VGA.hpp"
#include "VGA_themes.hpp"
#include "std/types.h"
#include "terminal.hpp"

extern "C" {
void kernel_main(void) {
  /* Initialize terminal interface */

  GDT_ptr gdt;
  gdt.init();
  IDT_ptr idt;
  idt.init();
  PIC::Remap();
  PIT::Init();
  IRQ::Init(); // unmasks IRQ1 itself, like PIT does for IRQ0

  Terminal terminal; // ~6.4 KB on the stack after the shadow removal
  terminal.Init();
  VGA::DAC::set_palette(0, VGA::DAC::THEMES::monokai_palette, 16);

  terminal.WriteString("[ INFO ] Hello, World!\n"
                       "[ INFO ] This is my first bootable kernel\n"
                       "[ INFO ] It called Charon o_o\n");

  /* Keyboard smoke test: echo every queued key event until the queue is
     empty, then halt and wait for the next IRQ1/IRQ0 to wake us. */
  terminal.WriteString("[ INFO ] Type! I will echo key events\n");
  for (;;) {
    IRQ::KeyEvent ev;
    if (IRQ::Poll(ev)) {
      static const char hex[] = "0123456789abcdef";
      terminal.WriteString("[ KEY ] 0x");
      terminal.PutChar(hex[ev.code >> 4]);
      terminal.PutChar(hex[ev.code & 0xF]);
      if (ev.ext)
        terminal.WriteString(" ext");
      terminal.WriteString(ev.pressed ? " down" : " up");
      terminal.WriteString("\n");
    } else {
      asm volatile("hlt");
    }
  }
}
}
