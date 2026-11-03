#ifndef TERMINAL_HPP
#define TERMINAL_HPP

#include "VGA.hpp"
#include "std/struct/ring.hpp"
#include "std/struct/stack.hpp"

#include "std/types.h"

struct Terminal {
  // state

  u8 column = 0; // `column` in Terminal is x in VGA
  u8 row = 0;    // `row` in `Terminal` is `y` in `VGA`

  Stack<VGA::Line, 20> below; // newer than the screen (only used while browsing)
  Ring<VGA::Line, 20> above;  // older than the screen (history)

  void Init(void);
  void Clear();
  void Flush(); // remove everything: history, browse state and screen
  void Write(const char *s, usize n);
  void WriteString(const char *s);
  void PutChar(char c);

  void SetAttribute(VGA::Color fg, VGA::Color bg);
  void SetPosition(u32 col, u32 row);

  void Activate();   // take the screen and show the cursor
  void Deactivate(); // release the screen and hide the cursor

  void ScrollContentUp();   // view toward newer text
  void ScrollContentDown(); // view toward older text

private:
  u8 attribute = VGA::DEFAULT_ATTRIBUTE;
  bool active = false; // is this one on screen?

  void snapToLive();
  void scrollLive();
  void syncCursor();
};

#endif
