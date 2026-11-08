// Keyboard.hpp
#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

#include "std/types.h"

namespace IRQ {

struct KeyEvent {
  u8  code;     // scancode without bit 7
  bool ext;     // came after 0xE0
  bool pressed; // make (true) or break (false)
};

void Init();
bool Poll(KeyEvent &out);
bool IsDown(u8 code, bool ext = false);

} // namespace IRQ

#endif
