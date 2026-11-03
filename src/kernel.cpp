#include "GDT.hpp"
#include "VGA.hpp"
#include "std/types.h"
#include "terminal.hpp"

#ifdef CHARON_STRESS
// Fill the screen fast, exercise wrap + scroll + the CP437 range.
static void stressTest(Terminal &terminal) {
  for (u32 i = 0; i < 4000; i++) {
    char c;
    if (i % 0xFF == 0)
      c = (char)(0x41 + (i >> 8));
    else
      c = (char)(0x61 + (i % 0xFF));
    terminal.PutChar(c);
  }
}
#endif

extern "C" {
void kernel_main(void) {
  /* Initialize terminal interface */

  GDT_ptr gdt;
  gdt.init();

  Terminal terminal; // ~6.4 KB on the stack after the shadow removal
  terminal.Init();

  terminal.WriteString("[ INFO ] Hello, World!\n"
                       "[ INFO ] This is my first bootable kernel\n"
                       "[ INFO ] It called Charon o_o\n");

#ifdef CHARON_STRESS
  stressTest(terminal);
#endif

  /* Nothing else to do yet: stop the CPU (interrupts are still disabled). */
  asm volatile("cli");
  for (;;)
    asm volatile("hlt");
}
}
