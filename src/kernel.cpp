
#include "GDT.hpp"
#include "VGA.hpp"
#include "std/types.h"
#include "terminal.hpp"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

extern "C" {
void kernel_main(void) {
  /* Initialize terminal interface */

  GTD_ptr gtd_ptr;
  gtd_ptr.init();

  Terminal terminal;
  terminal.Init();
  /* Newline support is left as an exercise. */

  terminal.WriteString("[ INFO ] Hello, World!\n"
                        "[ INFO ] This is my first bootable kernal\n"
                        "[ INFO ] It called Charon o_o");
  for (u16 i = 0;; i++) {
    if (i % 0xFF == 0) {
      char c = 0x41 + (i >> 8);
      terminal.PutChar(c);
    } else {
      char c = 0x61 + (i % 0xFF);
      terminal.PutChar(c);
    }
  }
}
}
