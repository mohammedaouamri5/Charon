#ifndef TERMINAL_HPP
#define TERMINAL_HPP

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct Terminal {
  size_t row, column;
  uint8_t color;
  uint16_t *buffer;
  void init(void);
  void set_color(uint8_t color);
  void put_entry_at(char c, uint8_t color, size_t x, size_t y);
  void put_char(char c);
  void write(const char *data, size_t size);
  void write_string(const char *data);
};

size_t strlen(const char *str);

#endif
