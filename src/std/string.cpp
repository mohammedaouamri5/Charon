#include "std/string.h"

namespace string {

// Length of a NUL-terminated string, terminator not counted.
usize length(const char *s) {
  usize n = 0;
  while (s[n])
    n++;
  return n;
}

} // namespace string
