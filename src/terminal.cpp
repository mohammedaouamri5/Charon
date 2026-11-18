#include "terminal.hpp"
#include "drivers/VGA.hpp"
#include "std/string.h"

// A row of blanks in the given attribute (used when new output scrolls in).
static VGA::Line blankLine(u8 attribute) {
  VGA::Line l;
  for (usize i = 0; i < VGA::WIDTH; i++)
    l.entry[i] = VGA::make_entry(' ', attribute);
  return l;
}

void Terminal::Init(void) {
  this->attribute = VGA::DEFAULT_ATTRIBUTE;
  VGA::cursor::init(); // known shape + visible before the first Clear
  this->Clear();
  this->Activate();
}

void Terminal::SetPosition(u32 col, u32 row) {
  // TODO: panic instead of clamping once panic exists
  this->column = col < VGA::WIDTH ? col : VGA::WIDTH - 1;
  this->row = row < VGA::HEIGHT ? row : VGA::HEIGHT - 1;
  VGA::cursor::move_to({this->column, this->row});
}

void Terminal::SetAttribute(VGA::Color fg, VGA::Color bg) {
  this->attribute = VGA::make_attribute(fg, bg);
}

// ---- history browsing -------------------------------------------------

// View toward newer text: content moves up.
void Terminal::ScrollContentUp() {
  VGA::Line incoming;
  if (not this->below.Pop(incoming))
    return; // already on the live screen
  this->above.Push(VGA::scroll_content_up(incoming));
}

// View toward older text: content moves down.
void Terminal::ScrollContentDown() {
  if (this->above.Empty() or this->below.Full())
    return;
  VGA::Line incoming;
  if (not this->above.PopNew(incoming))
    return;
  this->below.Push(VGA::scroll_content_down(incoming));
}

void Terminal::snapToLive() {
  while (not this->below.Empty())
    this->ScrollContentUp();
}

// Live output: the top row becomes history, a blank row enters at the bottom.
void Terminal::scrollLive() {
  this->snapToLive();
  this->above.Push(VGA::scroll_content_up(blankLine(this->attribute)));
}

// ---- output -----------------------------------------------------------Write

void Terminal::PutChar(char c) {
  this->snapToLive(); // new output while browsing: go back to live first

  switch (c) {
  case '\n':
    this->column = 0;
    this->row++;
    break;
  case '\r':
    this->column = 0;
    break;
  case '\t': {
    u32 next = (this->column + 8) & ~7u;
    while (this->column < next and this->column < VGA::WIDTH) {
      VGA::put_char({this->column, this->row}, ' ', this->attribute);
      this->column++;
    }
    break;
  }
  case '\b': // cursor only; erasing is the line discipline's job later
    if (this->column > 0)
      this->column--;
    break;
  default:
    if ((u8)c < 0x20)
      break; // skip other control characters
    VGA::put_char({this->column, this->row}, c, this->attribute);
    this->column++;
    break;
  }

  if (this->column >= VGA::WIDTH) {
    this->column = 0;
    this->row++;
  }
  if (this->row >= VGA::HEIGHT) {
    this->scrollLive();
    this->row = VGA::HEIGHT - 1;
  }
  this->syncCursor();
}

void Terminal::Activate() {
  this->active = true;
  this->syncCursor();
  VGA::cursor::show();
}

void Terminal::Deactivate() {
  this->active = false;
  VGA::cursor::hide(); // TODO : restore the screen once multiple terminals exist
}

void Terminal::syncCursor() {
  VGA::cursor::move_to({this->column, this->row});
}

void Terminal::Clear() {
  VGA::clear(VGA::make_entry(' ', this->attribute));
  this->SetPosition(0, 0);
}

// Remove everything: history, browse state and screen.
void Terminal::Flush() {
  this->above.Clear();
  this->below.Clear();
  this->Clear();
}

void Terminal::Write(const char *data, usize size) {
  for (usize i = 0; i < size; i++)
    this->PutChar(data[i]); // PutChar keeps the hardware cursor in sync
}

void Terminal::WriteU64(u64 value, u8 base) {
  if (base < 2 || base > 16)
    base = 10;

  char buf[20]; // enough for u64 in base 2
  int n = 0;

  do {
    u8 digit = value % base;
    buf[n++] = digit < 10 ? '0' + digit : 'a' + (digit - 10);
    value /= base;
  } while (value != 0);

  while (n > 0)
    this->PutChar(buf[--n]);
}

void Terminal::WriteHex(u64 value) {
  this->WriteString("0x");
  for (int shift = 60; shift >= 0; shift -= 4) {
    u8 digit = (value >> shift) & 0xF;
    this->PutChar(digit < 10 ? '0' + digit : 'a' + (digit - 10));
  }
}



void Terminal::WriteString(const char *data) {
  this->Write(data, string::length(data));
}
