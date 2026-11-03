// VGA.hpp
#ifndef VGA_HPP
#define VGA_HPP

#include "std/types.h"

namespace VGA {

// ---------------------------------------------------------------- constants
constexpr usize WIDTH = 80;
constexpr usize HEIGHT = 25;

// ---------------------------------------------------------------- colors
// The 16 text-mode colors, in VGA order (not ANSI order).
enum Color : u8 {
  COLOR_BLACK = 0,
  COLOR_BLUE = 1,
  COLOR_GREEN = 2,
  COLOR_CYAN = 3,
  COLOR_RED = 4,
  COLOR_MAGENTA = 5,
  COLOR_BROWN = 6,
  COLOR_LIGHT_GREY = 7,
  COLOR_DARK_GREY = 8,
  COLOR_LIGHT_BLUE = 9,
  COLOR_LIGHT_GREEN = 10,
  COLOR_LIGHT_CYAN = 11,
  COLOR_LIGHT_RED = 12,
  COLOR_LIGHT_MAGENTA = 13,
  COLOR_LIGHT_BROWN = 14,
  COLOR_WHITE = 15,
};

// attribute byte : bits 0-3 foreground, bits 4-6 background,
// bit 7 blink OR bright background (see mode::set_blink)
constexpr u8 make_attribute(Color fg, Color bg) {
  return (u8)((u8)fg | ((u8)bg << 4));
}

constexpr u8 DEFAULT_ATTRIBUTE = make_attribute(COLOR_LIGHT_GREY, COLOR_BLACK);

// entry : low byte = character (code page 437), high byte = attribute
constexpr u16 make_entry(char c, u8 attribute = DEFAULT_ATTRIBUTE) {
  return (u16)((u16)(u8)c | ((u16)attribute << 8));
}

constexpr u16 BLANK_ENTRY = make_entry(' ', DEFAULT_ATTRIBUTE);

// ---------------------------------------------------------------- geometry
// A position on the screen (column x, row y).
// The class does NOT check bounds, the drawing functions do.
class Point {
public:
  Point();
  Point(usize px, usize py);
  explicit Point(usize index); // linear cell index -> (x, y)

  usize x() const { return x_; }
  usize y() const { return y_; }

  // linear index in the buffer : y * WIDTH + x (plain math, no checks)
  usize offset() const;

  // checked construction, returns false and leaves `out` untouched
  // if (x, y) is off-screen
  static bool make(usize x, usize y, Point &out);

  Point add(const Point &rhs) const;

private:
  usize x_;
  usize y_;
};

struct Dimensions {
  usize width;
  usize height;
};

// A rectangle : top-left corner + size.
struct Rect {
  Point origin;
  Dimensions size;
};

// 6-bit components (0-63), the DAC ignores the upper two bits.
struct RGB {
  u8 r;
  u8 g;
  u8 b;
};

struct Line {
  u16 entry[WIDTH];
};

// ---------------------------------------------------------------- cells
// Out-of-range policy for the whole driver : draw what is visible,
// ignore the rest. Nothing here fails.

void put(Point at, u16 entry);
void put_char(Point at, char c, u8 attribute);
u16 get_entry(Point at);

// Fill a rectangle with one entry. Whole screen = clear, one row = blank a
// line.
void fill_rect(u16 entry, Rect area = {{0, 0}, {WIDTH, HEIGHT}});

// Copy a rectangle to a new top-left position (memmove semantics, overlap
// safe). Scroll = move_rect({{0,1},{WIDTH,HEIGHT-1}}, {0,0}) + fill last row.
void move_rect(Rect src, Point dest);

Line scroll_up();
Line scroll_down();

void snap_shot(u16 *buffer, Rect area);

// Copy a row-major array of entries (area.size.width * area.size.height of
// them) onto the screen at `area`.
void put_buffer(const u16 *src, Rect area);

// Thin helper : fill_rect over the whole screen.
void clear(u16 entry = BLANK_ENTRY);

Dimensions get_dimensions();

// Content moves up one row (the direction new output pushes it).
// Returns the row that fell off the top; `incoming` appears at the bottom.
Line scroll_content_up(const Line &incoming);
Line scroll_content_up();

// Content moves down one row. Returns the row that fell off the bottom.
Line scroll_content_down(const Line &incoming);

// ---------------------------------------------------------------- cursor
// Hardware cursor (CRTC registers). The hardware holds the state,
// the driver keeps no copy.
namespace cursor {

void init(u8 start = 13, u8 end = 15); // sets shape and enables the cursor
void move_to(Point to);
void move_by(i32 dx, i32 dy); // clamped to the screen
Point get_cursor_position();

void show();
void hide();
bool is_visible();

void set_shape(u8 start_scanline, u8 end_scanline);
void set_shape_underline();
void set_shape_block();

} // namespace cursor

// ---------------------------------------------------------------- mode
// Attribute controller settings.
namespace mode {

// true  : attribute bit 7 makes the character blink (8 background colors)
// false : attribute bit 7 is a bright background (16 background colors)
void set_blink(bool enable);

} // namespace mode

// ---------------------------------------------------------------- DAC
// Palette : changes what the 16 text colors look like.
namespace DAC {

void set_palette_color(u8 slot, RGB color);
RGB get_palette_color(u8 slot);
void set_palette(u8 first, const RGB *colors, usize count);
void reset_palette(); // standard VGA colors

} // namespace DAC

} // namespace VGA

#endif
