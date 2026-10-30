
#include "terminal.hpp"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

extern "C" {
void kernel_main(void) {
  /* Initialize terminal interface */
  Terminal terminal;
  terminal.init();
  /* Newline support is left as an exercise. */
  terminal.write_string("[ INFO ] Hello, World!\n"
                        "[ INFO ] This is my first bootable kernal\n"
                        "[ INFO ] It called Charon o_o");
}
}
