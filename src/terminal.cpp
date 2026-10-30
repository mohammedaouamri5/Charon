
#include "terminal.hpp"
#include "VGA.hpp"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
/* Hardware text mode color constants. */
size_t strlen(const char *str) {
  size_t len = 0;
  while (str[len])
    len++;
  return len;
}

void Terminal::init(void) {
  this->row = 0;
  this->column = 0;
  this->color = VGA_ENTRY_COLOR(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
  this->buffer = (uint16_t *)VGA_MEMORY;
  for (size_t y = 0; y < VGA_HEIGHT; y++) {
    for (size_t x = 0; x < VGA_WIDTH; x++) {
      const size_t index = y * VGA_WIDTH + x;
      this->buffer[index] = VGA_ENTRY('+', this->color);
    }
  }
}
void Terminal::set_color(uint8_t color) { this->color = color; }

void Terminal::put_entry_at(char c, uint8_t color, size_t x, size_t y) {
  const size_t index = y * VGA_WIDTH + x;
  this->buffer[index] = VGA_ENTRY(c, color);
}

void Terminal::put_char(char c) {
  if (c == '\n') {
    this->row += 1;
    this->column = 0;
  } else {
    this->put_entry_at(c, this->color, this->column, this->row);

    if (++this->column == VGA_WIDTH) {
      this->column = 0;
      if (++this->row == VGA_HEIGHT)
        this->row = 0;
    }
  }
}

void Terminal::write(const char *data, size_t size) {
  for (size_t i = 0; i < size; i++)
    this->put_char(data[i]);
}

void Terminal::write_string(const char *data) {
  this->write(data, strlen(data));
}
