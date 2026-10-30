
#ifndef VGA_HPP
#define VGA_HPP

#include <stdint.h>
enum VGA_COLOR_enum {
  VGA_COLOR_BLACK = 0,
  VGA_COLOR_BLUE = 1,
  VGA_COLOR_GREEN = 2,
  VGA_COLOR_CYAN = 3,
  VGA_COLOR_RED = 4,
  VGA_COLOR_MAGENTA = 5,
  VGA_COLOR_BROWN = 6,
  VGA_COLOR_LIGHT_GREY = 7,
  VGA_COLOR_DARK_GREY = 8,
  VGA_COLOR_LIGHT_BLUE = 9,
  VGA_COLOR_LIGHT_GREEN = 10,
  VGA_COLOR_LIGHT_CYAN = 11,
  VGA_COLOR_LIGHT_RED = 12,
  VGA_COLOR_LIGHT_MAGENTA = 13,
  VGA_COLOR_LIGHT_BROWN = 14,
  VGA_COLOR_WHITE = 15,
};
typedef enum VGA_COLOR_enum VGA_COLOR;

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

static inline uint8_t VGA_ENTRY_COLOR(VGA_COLOR fg, VGA_COLOR bg) {
  return fg | bg << 4;
}

static inline uint16_t VGA_ENTRY(uint8_t uc, uint8_t color) {
  return (uint16_t)uc | (uint16_t)color << 8;
}


#endif
