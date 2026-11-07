/*
 * =============================================================================
 *  VGA HARDWARE REFERENCE (text mode, color setup, 0x3Dx ports)
 * =============================================================================
 *
 *  THE PATTERN
 *  -----------
 *  VGA has many more registers than ports, so most register groups share a
 *  pair of ports:
 *
 *      INDEX port : write WHICH register you want
 *      DATA  port : read/write THAT register's value
 *
 *      outb(CRTC_INDEX, 0x0A);   // select register 0x0A (Cursor Start)
 *      outb(CRTC_DATA,  0x0E);   // write its value
 *
 *  Exceptions: the Attribute Controller (0x3C0) uses ONE port for both,
 *  and the DAC uses a write-index / data scheme (see below).
 *
 * -----------------------------------------------------------------------------
 *  PORT MAP
 * -----------------------------------------------------------------------------
 *  0x3C0  Attribute Controller: index AND data (a hidden flip-flop alternates
 *         between "next write is index" and "next write is data")
 *  0x3C1  Attribute Controller: read back the selected register
 *  0x3C2  Misc Output (write)
 *  0x3C4  Sequencer index
 *  0x3C5  Sequencer data
 *  0x3C6  DAC pixel mask (leave at 0xFF)
 *  0x3C7  DAC read index (write) / DAC state (read)
 *  0x3C8  DAC write index
 *  0x3C9  DAC data (R, G, B, 6 bits each, 3 consecutive accesses per entry)
 *  0x3CC  Misc Output (read back of 0x3C2)
 *  0x3CE  Graphics Controller index
 *  0x3CF  Graphics Controller data
 *  0x3D4  CRTC index
 *  0x3D5  CRTC data
 *  0x3DA  Input Status 1 (read). Reading it RESETS the 0x3C0 flip-flop.
 *
 *  Monochrome mirrors: 0x3B4 / 0x3B5 / 0x3BA. Misc Output bit 0 selects which
 *  set the card answers on. Normal color setups use the 0x3Dx set.
 *
 * -----------------------------------------------------------------------------
 *  CRTC REGISTERS (index 0x3D4, data 0x3D5)
 * -----------------------------------------------------------------------------
 *  Registers you will actually use:
 *
 *  0x0A  Cursor Start
 *          bits 0-4 : first scanline of the cursor
 *          bit  5   : 1 = cursor DISABLED
 *          bits 6-7 : reserved
 *  0x0B  Cursor End
 *          bits 0-4 : last scanline of the cursor
 *          bits 5-6 : cursor skew (shifts cursor right by N cells)
 *          bit  7   : reserved
 *  0x0C  Start Address High  \  which cell is drawn at the top-left of the
 *  0x0D  Start Address Low   /  screen. Changing it scrolls the whole screen
 *                               with no copying (hardware scroll).
 *  0x0E  Cursor Location High \ cursor position as a CELL INDEX (y*80 + x),
 *  0x0F  Cursor Location Low  / NOT a byte offset.
 *
 *  Display timing, leave alone (BIOS/GRUB already set these):
 *
 *  0x00  Horizontal Total             0x0A-0x0F  see above
 *  0x01  End Horizontal Display       0x10  Vertical Retrace Start
 *  0x02  Start Horizontal Blanking    0x11  Vertical Retrace End
 *  0x03  End Horizontal Blanking            (bit 7 write-protects regs 0-7)
 *  0x04  Start Horizontal Retrace     0x12  Vertical Display End
 *  0x05  End Horizontal Retrace       0x13  Offset (distance between rows)
 *  0x06  Vertical Total               0x14  Underline Location
 *  0x07  Overflow (extra high bits)   0x15  Start Vertical Blanking
 *  0x08  Preset Row Scan              0x16  End Vertical Blanking
 *  0x09  Maximum Scan Line            0x17  Mode Control
 *        (char height - 1, so 15)     0x18  Line Compare (split screen)
 *
 *  Cursor shape examples (scanlines 0-15 on the usual 8x16 font):
 *      underline : Start = 14,   End = 15
 *      block     : Start = 0,    End = 15
 *      hidden    : Start = 0x20 (bit 5 set)
 *  When changing shape or visibility, READ the register first and keep the
 *  bits you are not touching (disable bit, skew), then write it back.
 *
 *  Example of a bad value: Start = 0x40, End = 0x40 gives a 1-scanline cursor
 *  at the top of the cell, with skew = 2 in End (bits 5-6 = 10b), which is
 *  almost never intended.
 *
 * -----------------------------------------------------------------------------
 *  ATTRIBUTE CONTROLLER REGISTERS (port 0x3C0 write, 0x3C1 read)
 * -----------------------------------------------------------------------------
 *  0x00-0x0F  Palette: for each of the 16 text colors, which DAC entry it uses.
 *             This is why the slot->DAC table exists:
 *               slots 0-5  -> DAC 0x00-0x05
 *               slot  6    -> DAC 0x14   (brown)
 *               slot  7    -> DAC 0x07
 *               slots 8-15 -> DAC 0x38-0x3F
 *  0x10       Mode Control
 *               bit 0 : 1 = graphics mode, 0 = text mode
 *               bit 3 : 1 = bit 7 of attribute BLINKS (8 background colors)
 *                       0 = bit 7 is a bright background bit (16 colors)
 *  0x11       Overscan (border) color
 *  0x12       Color Plane Enable
 *  0x13       Horizontal Pixel Panning
 *  0x14       Color Select
 *
 *  Access sequence (read-modify-write of a register):
 *      1. inb(0x3DA)               reset the flip-flop to "next write = index"
 *      2. outb(0x3C0, index)       select register (see trap 2 below)
 *      3. val = inb(0x3C1)         read current value
 *      4. outb(0x3C0, val | bit)   write the new value (flip-flop: data)
 *      5. inb(0x3DA)               reset flip-flop again
 *      6. outb(0x3C0, 0x20)        re-enable video (see trap 2)
 *
 *  TRAP 1: forgetting to read 0x3DA first. You don't know whether your next
 *          write is treated as an index or as data.
 *  TRAP 2: bit 5 (0x20) of the INDEX byte is "palette address source".
 *          If it is 0 when you finish, the palette is handed to the CPU and
 *          the SCREEN GOES BLANK. Always finish by writing 0x20 to 0x3C0.
 *          (Also OR 0x20 into the index you write in step 2.)
 *
 * -----------------------------------------------------------------------------
 *  DAC (palette colors)
 * -----------------------------------------------------------------------------
 *  Write an entry:  outb(0x3C8, entry); then outb(0x3C9, R), G, B (0-63 each).
 *  Read an entry:   outb(0x3C7, entry); then inb(0x3C9) three times: R, G, B.
 *  The entry number auto-increments after each R,G,B triple, so a whole
 *  palette can be streamed without re-writing the index.
 *  Values are 6 bits per channel (0-63), NOT 8.
 *  The index you give is a DAC entry, NOT a text color slot: translate the
 *  slot through the slot->DAC table first.
 *
 * -----------------------------------------------------------------------------
 *  SEQUENCER (0x3C4 index / 0x3C5 data)
 * -----------------------------------------------------------------------------
 *  0x00  Reset
 *  0x01  Clocking Mode (bit 0: 8 vs 9 pixel-wide chars, bit 5: screen off)
 *  0x02  Map Mask (which memory planes CPU writes go to)
 *  0x03  Character Map Select (which fonts are used)
 *  0x04  Memory Mode
 *  In text mode, plane 0 holds characters, plane 1 attributes, plane 2 the
 *  font. Loading a custom font means touching these plus the Graphics
 *  Controller. Not needed until much later.
 *
 * -----------------------------------------------------------------------------
 *  GRAPHICS CONTROLLER (0x3CE index / 0x3CF data)
 * -----------------------------------------------------------------------------
 *  0x00-0x05, 0x07, 0x08  Graphics-mode data paths (set/reset, read map,
 *                         mode, color compare, mask). Ignore for text mode.
 *  0x06  Miscellaneous
 *          bit 0    : 1 = graphics mode, 0 = text mode
 *          bits 2-3 : memory map location
 *                       00 = 0xA0000 (128 KB)
 *                       01 = 0xA0000 (64 KB)
 *                       10 = 0xB0000 (32 KB)
 *                       11 = 0xB8000 (32 KB)   <-- why text mode lives here
 *
 * -----------------------------------------------------------------------------
 *  MISC OUTPUT (0x3C2 write, 0x3CC read)
 * -----------------------------------------------------------------------------
 *  bit 0     : I/O address select (1 = 0x3Dx color ports, 0 = 0x3Bx mono)
 *  bit 1     : 1 = CPU may access video RAM
 *  bits 2-3  : clock select (00 = 25 MHz / 640 px, 01 = 28 MHz / 720 px)
 *  bit 5     : page select (odd/even)
 *  bits 6-7  : sync polarity (together they signal the line count to the
 *              monitor: 350, 400 or 480 lines)
 *
 * -----------------------------------------------------------------------------
 *  WHAT THIS DRIVER TOUCHES
 * -----------------------------------------------------------------------------
 *  Move cursor ............ CRTC 0x0E, 0x0F
 *  Show / hide / shape .... CRTC 0x0A, 0x0B
 *  Hardware scroll ........ CRTC 0x0C, 0x0D (optional)
 *  Blink vs bright bg ..... AC 0x10 (via 0x3DA reset + 0x3C0/0x3C1)
 *  Custom colors .......... DAC 0x3C7/0x3C8/0x3C9 + slot->DAC table
 *  Fonts .................. Sequencer + Graphics Controller (much later)
 *  Everything else is mode setting, already done by BIOS/GRUB.
 * =============================================================================
 */

// VGA.cpp

#include "VGA.hpp"
#include "VGA_themes.hpp"
#include "std/io.hpp"
#include "std/types.h"

namespace VGA {

// volatile: the compiler must not merge/drop/reorder writes to the screen
static volatile u16 *VGA_buff = (volatile u16 *)0xB8000;

// ---------------------------------------------------------------- point
Point::Point() : x_(0), y_(0) {}

Point::Point(usize px, usize py) : x_(px), y_(py) {}

Point::Point(usize index) {
  x_ = index % WIDTH;
  y_ = index / WIDTH;
}

// plain math only, the callers decide what to do with an out-of-range point
usize Point::offset() const { return y_ * WIDTH + x_; }

bool Point::make(usize x, usize y, Point &out) {
  if (x >= WIDTH || y >= HEIGHT)
    return false;
  out = Point(x, y);
  return true;
}

Point Point::add(const Point &other) const {
  return Point(this->x_ + other.x_, this->y_ + other.y_);
}

// ---------------------------------------------------------------- helpers
static bool in_bounds(Point p) { return p.x() < WIDTH && p.y() < HEIGHT; }

// Cut a rect so it fits on the screen. Origin off-screen => empty rect.
// Policy for the whole driver: draw what is visible, ignore the rest.
static Rect clip(Rect r) {
  if (r.origin.x() >= WIDTH || r.origin.y() >= HEIGHT)
    return {r.origin, {0, 0}};

  usize max_w = WIDTH - r.origin.x();
  usize max_h = HEIGHT - r.origin.y();
  if (r.size.width > max_w)
    r.size.width = max_w;
  if (r.size.height > max_h)
    r.size.height = max_h;
  return r;
}

// ----------------------------------------------------------------   cells
void put(Point at, u16 entry) {
  if (!in_bounds(at)) // TODO : panic once we have one
    return;
  VGA_buff[at.offset()] = entry;
}

void put_char(Point at, char c, u8 attribute) {
  put(at, make_entry(c, attribute));
}

u16 get_entry(Point at) {
  if (!in_bounds(at)) // TODO : panic once we have one
    return BLANK_ENTRY;
  return VGA_buff[at.offset()];
}

// Fill a rectangle with one entry. Whole screen = clear, one row = blank a
// line.
void fill_rect(u16 entry, Rect area) {
  area = clip(area);

  // TODO : change it with some asm code that have better instruction for
  // filling memory with the same value after std/mm
  for (usize y = area.origin.y(); y < area.origin.y() + area.size.height; y++) {
    for (usize x = area.origin.x(); x < area.origin.x() + area.size.width;
         x++) {
      put({x, y}, entry);
    }
  }
}

// Copy a rectangle of cells to a new top-left position.
// Must handle overlapping source/destination (memmove semantics):
// copy direction depends on whether dest is before or after src.
void move_rect(Rect src, Point dest) {
  src = clip(src);
  if (dest.x() >= WIDTH || dest.y() >= HEIGHT)
    return;

  // shrink so the destination stays on screen too
  usize w = src.size.width;
  usize h = src.size.height;
  if (w > WIDTH - dest.x())
    w = WIDTH - dest.x();
  if (h > HEIGHT - dest.y())
    h = HEIGHT - dest.y();

  usize sx = src.origin.x();
  usize sy = src.origin.y();
  usize dx = dest.x();
  usize dy = dest.y();

  // TODO : change it with some asm code that have better instruction for
  // moving memory around after std/mm
  if (dy < sy || (dy == sy && dx <= sx)) {
    // dest is before src : top to bottom, left to right
    for (usize y = 0; y < h; y++)
      for (usize x = 0; x < w; x++)
        put({dx + x, dy + y}, get_entry({sx + x, sy + y}));
  } else {
    // dest is after src : bottom to top, right to left
    for (usize y = h; y-- > 0;)
      for (usize x = w; x-- > 0;)
        put({dx + x, dy + y}, get_entry({sx + x, sy + y}));
  }
}

// Copy a row-major array of entries (area.size.width * area.size.height of
// them) onto the screen at `area`.
void put_buffer(const u16 *src, Rect area) {
  if (src == nullptr) // TODO : panic once we have one
    return;

  usize stride = area.size.width; // row length of the SOURCE array
  Rect visible = clip(area);

  for (usize y = 0; y < visible.size.height; y++) {
    for (usize x = 0; x < visible.size.width; x++) {
      put({area.origin.x() + x, area.origin.y() + y}, src[y * stride + x]);
    }
  }
}

void snap_shot(u16 *buffer, Rect area) {
  if (buffer == nullptr) // TODO : panic once we have one
    return;

  for (usize y = 0; y < area.size.height; y++) {
    for (usize x = 0; x < area.size.width; x++) {
      buffer[y * area.size.width + x] =
          get_entry({x + area.origin.x(), y + area.origin.y()});
    }
  }
}
// Thin helper: fill_rect over the whole screen.
void clear(u16 entry) { fill_rect(entry, {{0, 0}, {WIDTH, HEIGHT}}); }

Dimensions get_dimensions() { return {WIDTH, HEIGHT}; }


// Content moves up one row (the direction new output pushes it).
// Returns the row that fell off the top; `incoming` appears at the bottom.
Line scroll_content_up(const Line &incoming) {
  Line out;
  for (usize x = 0; x < WIDTH; x++)
    out.entry[x] = get_entry({x, 0});
  move_rect({{0, 1}, {WIDTH, HEIGHT - 1}}, {0, 0}); // you already have this
  for (usize x = 0; x < WIDTH; x++)
    put({x, HEIGHT - 1}, incoming.entry[x]);
  return out;
}


Line scroll_content_up() {
  Line out;
  for (usize x = 0; x < WIDTH; x++)
    out.entry[x] = get_entry({x, 0});
  move_rect({{0, 1}, {WIDTH, HEIGHT - 1}}, {0, 0}); // you already have this
  fill_rect(BLANK_ENTRY, {{0, HEIGHT - 1}, {WIDTH, 1}});
  return out;
}

// Content moves down one row. Returns the row that fell off the bottom.
Line scroll_content_down(const Line &incoming) {
  Line out;
  for (usize x = 0; x < WIDTH; x++)
    out.entry[x] = get_entry({x, HEIGHT - 1});
  move_rect({{0, 0}, {WIDTH, HEIGHT - 1}}, {0, 1});
  for (usize x = 0; x < WIDTH; x++)
    put({x, 0}, incoming.entry[x]);
  return out;
}

// ---------------------------------------------------------------- cursor
// Hardware cursor, driven through CRTC registers (ports 0x3D4/0x3D5).
// Independent from the text buffer. Position is stored in the hardware,
// so the driver keeps no copy.
namespace cursor {

void init(u8 start, u8 end) {
  using namespace io;
  outb(0x3D4, 0x0A);
  outb(0x3D5, (inb(0x3D5) & 0xC0) | start); // also clears the disable bit

  outb(0x3D4, 0x0B);
  outb(0x3D5, (inb(0x3D5) & 0xE0) | end);
}

void move_to(Point to) {
  using namespace io;
  if (!in_bounds(to))
    return;
  u16 pos = to.offset();
  outb(0x3D4, 0x0F);
  outb(0x3D5, (u8)(pos & 0xFF));

  outb(0x3D4, 0x0E);
  outb(0x3D5, (u8)((pos >> 8) & 0xFF));
}

Point get_cursor_position() {
  using namespace io;
  u16 pos = 0;
  outb(0x3D4, 0x0F);
  pos |= inb(0x3D5);
  outb(0x3D4, 0x0E);
  pos |= ((u16)inb(0x3D5)) << 8;
  return Point((usize)pos);
}

// Only bit 5 of register 0x0A controls visibility.
// Read-modify-write so the cursor shape is kept.
void hide() {
  using namespace io;
  outb(0x3D4, 0x0A);
  outb(0x3D5, inb(0x3D5) | 0x20);
}

void show() {
  using namespace io;
  outb(0x3D4, 0x0A);
  outb(0x3D5, inb(0x3D5) & ~0x20);
}

bool is_visible() {
  using namespace io;
  outb(0x3D4, 0x0A);
  return (inb(0x3D5) & 0x20) == 0; // bit set = disabled
}

// signed delta so it can go left/up, result is clamped to the screen
void move_by(i32 dx, i32 dy) {
  // todo replace it by better asm function and call it
  Point cur = get_cursor_position();
  i32 nx = (i32)cur.x() + dx;
  i32 ny = (i32)cur.y() + dy;

  if (nx < 0)
    nx = 0;
  if (ny < 0)
    ny = 0;
  if (nx >= (i32)WIDTH)
    nx = WIDTH - 1;
  if (ny >= (i32)HEIGHT)
    ny = HEIGHT - 1;

  move_to(Point((usize)nx, (usize)ny));
}

void set_shape(u8 start_scanline, u8 end_scanline) {
  using namespace io;
  outb(0x3D4, 0x0A);
  outb(0x3D5, (inb(0x3D5) & 0xC0) | start_scanline);
  outb(0x3D4, 0x0B);
  outb(0x3D5, (inb(0x3D5) & 0xE0) | end_scanline);
}

void set_shape_underline() { set_shape(14, 15); }

void set_shape_block() { set_shape(0, 15); }

} // namespace cursor

// ---------------------------------------------------------------- mode
// Attribute controller settings (NOT the CRTC, different chip).
namespace mode {

// Mode control register = index 0x10, bit 3 = blink enable.
// Bit set   : bit 7 of the attribute blinks the character.
// Bit clear : bit 7 of the attribute is a bright background (16 bg colors).
void set_blink(bool enable) {
  using namespace io;
  inb(0x3DA);               // reset the index/data flip-flop
  outb(0x3C0, 0x10 | 0x20); // select register 0x10, keep the palette enabled
  u8 value = inb(0x3C1);    // read current value
  outb(0x3C0, (value & 0xF7) | (enable ? 0x08 : 0x00)); // write back

  inb(0x3DA);        // reset the flip-flop again
  outb(0x3C0, 0x20); // palette address source back to the video hardware
}

} // namespace mode

// Components are 6 bits (0-63), higher bits are masked off.
namespace DAC {

constexpr u8 slot_to_DAC_table[16] = {0x0,  0x1,  0x2,  0x3,  0x4,  0x5,
                                      0x14, 0x7,  0x38, 0x39, 0x3A, 0x3B,
                                      0x3C, 0x3D, 0x3E, 0x3F};

void set_palette_color(u8 slot, RGB color) {
  using namespace io;
  if (slot >= 16)
    return;
  outb(0x3C8, slot_to_DAC_table[slot]);
  outb(0x3C9, color.r & 0x3F);
  outb(0x3C9, color.g & 0x3F);
  outb(0x3C9, color.b & 0x3F);
}

RGB get_palette_color(u8 slot) {
  RGB color = {0, 0, 0};
  using namespace io;
  if (slot >= 16)
    return color;
  outb(0x3C7, slot_to_DAC_table[slot]);
  color.r = inb(0x3C9);
  color.g = inb(0x3C9);
  color.b = inb(0x3C9);
  return color;
}

// Sets `count` consecutive slots starting at `first`.
void set_palette(u8 first, const RGB *colors, usize count) {
  for (usize i = 0; i < count; i++) {
    set_palette_color(first + i, colors[i]);
  }
}

// Restore the standard VGA colors.
void reset_palette() { set_palette(0, THEMES::standard_palette, 16); }

} // namespace DAC

} // namespace VGA
