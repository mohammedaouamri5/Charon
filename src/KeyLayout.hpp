/*
================================================================================
 KeyLayout.hpp - US QWERTY layout for PS/2 scancode set 2

 Pure table: set-2 scancode + modifier state -> ASCII. Kept in its own header
 so the shell (kernel.cpp) stays about control flow, not about key codes.

 GO-STYLE: exported names PascalCase.
================================================================================
*/

#ifndef KEYLAYOUT_HPP
#define KEYLAYOUT_HPP

#include "std/types.h"

namespace KeyLayout {

// set-2 scancode -> ASCII. 0 = not printable (or handled elsewhere).
inline char DecodeKey(u8 code, bool shift, bool caps) {
  bool upper = caps ^ shift;

  switch (code) {
  /* letters */
  case 0x15: return upper ? 'Q' : 'q'; // Q
  case 0x1D: return upper ? 'W' : 'w'; // W
  case 0x24: return upper ? 'E' : 'e'; // E
  case 0x2D: return upper ? 'R' : 'r'; // R
  case 0x2C: return upper ? 'T' : 't'; // T
  case 0x35: return upper ? 'Y' : 'y'; // Y
  case 0x3C: return upper ? 'U' : 'u'; // U
  case 0x43: return upper ? 'I' : 'i'; // I
  case 0x44: return upper ? 'O' : 'o'; // O
  case 0x4D: return upper ? 'P' : 'p'; // P
  case 0x1A: return upper ? 'Z' : 'z'; // Z
  case 0x22: return upper ? 'X' : 'x'; // X
  case 0x21: return upper ? 'C' : 'c'; // C
  case 0x2A: return upper ? 'V' : 'v'; // V
  case 0x32: return upper ? 'B' : 'b'; // B
  case 0x31: return upper ? 'N' : 'n'; // N
  case 0x3A: return upper ? 'M' : 'm'; // M
  case 0x1C: return upper ? 'A' : 'a'; // A
  case 0x1B: return upper ? 'S' : 's'; // S
  case 0x23: return upper ? 'D' : 'd'; // D
  case 0x2B: return upper ? 'F' : 'f'; // F
  case 0x34: return upper ? 'G' : 'g'; // G
  case 0x33: return upper ? 'H' : 'h'; // H
  case 0x3B: return upper ? 'J' : 'j'; // J
  case 0x42: return upper ? 'K' : 'k'; // K
  case 0x4B: return upper ? 'L' : 'l'; // L
  /* digits */
  case 0x16: return shift ? '!' : '1'; // 1
  case 0x1E: return shift ? '@' : '2'; // 2
  case 0x26: return shift ? '#' : '3'; // 3
  case 0x25: return shift ? '$' : '4'; // 4
  case 0x2E: return shift ? '%' : '5'; // 5
  case 0x36: return shift ? '^' : '6'; // 6
  case 0x3D: return shift ? '&' : '7'; // 7
  case 0x3E: return shift ? '*' : '8'; // 8
  case 0x46: return shift ? '(' : '9'; // 9
  case 0x45: return shift ? ')' : '0'; // 0
  /* punctuation */
  case 0x29: return ' ';               // Space
  case 0x0E: return shift ? '~' : '`'; // Backtick
  case 0x4E: return shift ? '_' : '-'; // -
  case 0x55: return shift ? '+' : '='; // =
  case 0x54: return shift ? '{' : '['; // [
  case 0x5B: return shift ? '}' : ']'; // ]
  case 0x5D: return shift ? '|' : '\\';// backslash (ANSI/ISO)
  case 0x4C: return shift ? ':' : ';'; // ;
  case 0x52: return shift ? '"' : '\'';// '
  case 0x41: return shift ? '<' : ','; // ,
  case 0x49: return shift ? '>' : '.'; // .
  case 0x4A: return shift ? '?' : '/'; // /
  default: return 0;                   // F-keys, modifiers, dead ends
  }
  return 0;
}

} // namespace KeyLayout

#endif
