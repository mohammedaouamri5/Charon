

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


// Amber Phosphor Terminal Theme (Classic monochrome amber look)
constexpr RGB amber_palette[16] = {
    {0x00, 0x00, 0x00}, // 0  black background
    {0x1A, 0x0D, 0x00}, // 1  dim amber
    {0x26, 0x13, 0x00}, // 2  dark amber
    {0x33, 0x1A, 0x00}, // 3  medium-dark amber
    {0x3F, 0x20, 0x00}, // 4  amber red-tint
    {0x3F, 0x26, 0x00}, // 5  muted amber
    {0x3F, 0x2B, 0x00}, // 6  bronze
    {0x3F, 0x33, 0x10}, // 7  light amber (normal text)
    {0x20, 0x10, 0x05}, // 8  dark gray equivalent
    {0x3F, 0x2A, 0x0A}, // 9  bright amber tint
    {0x3F, 0x33, 0x00}, // 10 warm amber
    {0x3F, 0x38, 0x1A}, // 11 pale amber cyan equivalent
    {0x3F, 0x20, 0x10}, // 12 amber red equivalent
    {0x3F, 0x30, 0x20}, // 13 amber magenta equivalent
    {0x3F, 0x3A, 0x2A}, // 14 bright yellow-amber
    {0x3F, 0x3F, 0x3F}, // 15 full white/bright amber
};

// Green Phosphor Terminal Theme (Classic monochrome green look)
constexpr RGB green_palette[16] = {
    {0x00, 0x00, 0x00}, // 0  black background
    {0x00, 0x10, 0x00}, // 1  dim green
    {0x00, 0x1A, 0x00}, // 2  dark green
    {0x00, 0x26, 0x00}, // 3  medium green
    {0x05, 0x2B, 0x05}, // 4  muted forest green
    {0x0A, 0x30, 0x0A}, // 5  soft green
    {0x10, 0x35, 0x10}, // 6  olive green
    {0x15, 0x3F, 0x15}, // 7  light green (normal text)
    {0x08, 0x15, 0x08}, // 8  dark gray equivalent
    {0x1A, 0x3F, 0x1A}, // 9  bright green tint
    {0x20, 0x3F, 0x20}, // 10 vibrant green
    {0x2A, 0x3F, 0x2A}, // 11 pale green
    {0x20, 0x35, 0x20}, // 12 muted red equivalent
    {0x25, 0x3A, 0x25}, // 13 muted magenta equivalent
    {0x33, 0x3F, 0x33}, // 14 yellow-green highlight
    {0x3F, 0x3F, 0x3F}, // 15 full white/bright green
};

// Dark Retro / Cyberpunk Palette
constexpr RGB cyberpunk_palette[16] = {
    {0x04, 0x02, 0x09}, // 0  deep dark void
    {0x00, 0x2A, 0x3A}, // 1  deep cyan/blue
    {0x10, 0x35, 0x10}, // 2  neon green
    {0x00, 0x3A, 0x3A}, // 3  cyan
    {0x3F, 0x0A, 0x1A}, // 4  hot pink/red
    {0x2A, 0x00, 0x2A}, // 5  purple
    {0x3F, 0x25, 0x00}, // 6  dark orange
    {0x2A, 0x2A, 0x30}, // 7  cool gray
    {0x10, 0x10, 0x18}, // 8  charcoal
    {0x00, 0x3F, 0x3F}, // 9  bright cyan
    {0x15, 0x3F, 0x15}, // 10 light green
    {0x15, 0x3F, 0x3F}, // 11 bright light cyan
    {0x3F, 0x15, 0x2A}, // 12 magenta-red
    {0x3F, 0x15, 0x3F}, // 13 light magenta
    {0x3F, 0x3F, 0x00}, // 14 electric yellow
    {0x3F, 0x3F, 0x3F}, // 15 white
};

// Monokai Theme
constexpr RGB monokai_palette[16] = {
    {0x0A, 0x0A, 0x08}, // 0  black / dark bg
    {0x19, 0x36, 0x3F}, // 1  blue
    {0x29, 0x38, 0x0B}, // 2  green
    {0x19, 0x38, 0x3A}, // 3  cyan
    {0x3E, 0x09, 0x1C}, // 4  red
    {0x2B, 0x20, 0x3F}, // 5  magenta
    {0x39, 0x36, 0x1D}, // 6  brown / yellow-dim
    {0x35, 0x35, 0x30}, // 7  light gray
    {0x1C, 0x1D, 0x18}, // 8  dark gray
    {0x26, 0x3D, 0x3F}, // 9  light blue
    {0x33, 0x3F, 0x14}, // 10 light green
    {0x25, 0x3F, 0x3E}, // 11 light cyan
    {0x3F, 0x1C, 0x2B}, // 12 light red
    {0x35, 0x2C, 0x3F}, // 13 light magenta
    {0x3F, 0x3D, 0x2A}, // 14 yellow
    {0x3E, 0x3E, 0x3B}, // 15 white / foreground
};




// Guard against 8-bit values sneaking back in (the DAC only takes 0-63).
constexpr bool palette_in_range(const RGB *palette) {
  for (usize i = 0; i < 16; i++)
    if (palette[i].r > 63 || palette[i].g > 63 || palette[i].b > 63)
      return false;
  return true;
} // <- this brace was missing

static_assert(palette_in_range(standard_palette),
              "Standard DAC colors must be 6-bit (0-63)");
static_assert(palette_in_range(amber_palette),
              "Amber DAC colors must be 6-bit (0-63)");
static_assert(palette_in_range(green_palette),
              "Green DAC colors must be 6-bit (0-63)");
static_assert(palette_in_range(cyberpunk_palette),
              "Cyberpunk DAC colors must be 6-bit (0-63)");
static_assert(palette_in_range(monokai_palette),
              "Monokai DAC colors must be 6-bit (0-63)");

} // namespace THEMES
} // namespace DAC
} // namespace VGA

#endif
