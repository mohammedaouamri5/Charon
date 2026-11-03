#include "GDT.hpp"
#include "VGA.hpp"
#include "std/types.h"
#include "terminal.hpp"

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

  /* Nothing else to do yet: stop the CPU (interrupts are still disabled). */
  asm volatile("cli");
  for (;;)
    asm volatile("hlt");
}
}
