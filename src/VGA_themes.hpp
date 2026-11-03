

#ifndef VGA_THEMES
#define VGA_THEMES
#include "VGA.hpp"
namespace VGA {
namespace DAC {
namespace THEMES {
// DAC values are 6 bits per channel (0-63), NOT 8 bits.
constexpr RGB standard_palette[16] = {
    {0x00, 0x00, 0x00}, // 0  black
    {0x00, 0x00, 0x2A}, // 1  blue
    {0x00, 0x2A, 0x00}, // 2  green
    {0x00, 0x2A, 0x2A}, // 3  cyan
    {0x2A, 0x00, 0x00}, // 4  red
    {0x2A, 0x00, 0x2A}, // 5  magenta
    {0x2A, 0x15, 0x00}, // 6  brown
    {0x2A, 0x2A, 0x2A}, // 7  light gray
    {0x15, 0x15, 0x15}, // 8  dark gray
    {0x15, 0x15, 0x3F}, // 9  light blue
    {0x15, 0x3F, 0x15}, // 10 light green
    {0x15, 0x3F, 0x3F}, // 11 light cyan
    {0x3F, 0x15, 0x15}, // 12 light red
    {0x3F, 0x15, 0x3F}, // 13 light magenta
    {0x3F, 0x3F, 0x15}, // 14 yellow
    {0x3F, 0x3F, 0x3F}, // 15 white
};

// Guard against 8-bit values sneaking back in (the DAC only takes 0-63).
constexpr bool palette_in_range() {
  for (usize i = 0; i < 16; i++)
    if (standard_palette[i].r > 63 || standard_palette[i].g > 63 ||
        standard_palette[i].b > 63)
      return false;
  return true;
}
STATIC_ASSERT(palette_in_range(), "DAC colors must be 6-bit (0-63)");
} // namespace THEMES
} // namespace DAC
} // namespace VGA

#endif
