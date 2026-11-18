#ifndef FMT_HPP
#define FMT_HPP

#include "std/string.h"
#include "std/types.h"

namespace FMT {

template <typename S>
concept BulkSink = requires(S &s, const char *p, usize n) { s.Write(p, n); };

template <typename S>
concept CharSink = requires(S &s, char c) { s.Put(c); };

template <typename S> void Emit(S &sink, const char *p, usize n) {
  if constexpr (BulkSink<S>) {
    sink.Write(p, n); // one call for a whole field
  } else {
    static_assert(CharSink<S>,
                  "fmt: sink needs Write(const char*, usize) or Put(char)");
    for (usize i = 0; i < n; ++i)
      sink.Put(p[i]);
  }
}

// Compile-time error markers. Declared, never defined: a call during
// constant evaluation fails the build and the name appears in the error.
void FormatErrorMalformedField();
void FormatErrorStrayBrace();
void FormatErrorArgumentCount();
void FormatErrorUnsupportedArgument();
void FormatErrorVerbMismatch();
void FormatErrorZeroPadding();

// ---------- Type helpers ----------

template <typename T> struct Id {
  using type = T;
};

template <typename T> struct IsPointer {
  static constexpr bool value = false;
};
template <typename T> struct IsPointer<T *> {
  static constexpr bool value = true;
};

template <typename T> struct PointeeOf {
  using type = void;
};
template <typename T> struct PointeeOf<T *> {
  using type = T;
};

enum Kind { INTEGER, CHARACTER, STRING, POINTER, UNSUPPORTED };

template <typename T> constexpr Kind KindOf() {
  if constexpr (IsPointer<T>::value) {
    using P = typename PointeeOf<T>::type;
    if constexpr (__is_same(P, char) || __is_same(P, const char))
      return STRING;
    else
      return POINTER;
  } else if constexpr (__is_same(T, char)) {
    return CHARACTER;
  } else if constexpr (__is_same(T, bool)) {
    return UNSUPPORTED;
  } else if constexpr (requires(T t) { t & 1; }) {
    return INTEGER;
  } else {
    return UNSUPPORTED;
  }
}

// ---------- Format fields: {<zeros><verb>} ----------

// x d b = integer, c = char, s = string, p = address
enum Form { HEX, DEC, BIN, CHR, STR, PTR, ERR };

struct Field {
  Form form;
  usize zeros;
};

// Parses one field starting at s[i] == '{'. Advances i past the '}'.
constexpr bool ParseField(const char *s, usize &i, Field &out) {
  if (s[i] != '{')
    return false;
  ++i;
  usize zeros = 0;
  while (s[i] >= '0' && s[i] <= '9') {
    zeros = zeros * 10 + static_cast<usize>(s[i] - '0');
    ++i;
  }
  Form form = ERR;
  switch (s[i]) {
  case 'x': form = HEX; break;
  case 'd': form = DEC; break;
  case 'b': form = BIN; break;
  case 'c': form = CHR; break;
  case 's': form = STR; break;
  case 'p': form = PTR; break;
  default:  return false;
  }
  ++i;
  if (s[i] != '}')
    return false;
  ++i;
  out = Field{form, zeros};
  return true;
}

constexpr bool Allowed(Form form, Kind kind) {
  switch (form) {
  case HEX:
  case DEC:
  case BIN: return kind == INTEGER;
  case CHR: return kind == CHARACTER;
  case STR: return kind == STRING;
  case PTR: return kind == POINTER || kind == STRING;
  default:  return false;
  }
}

// ---------- Format string, checked at compile time ----------

template <typename... Args> struct Checked {
  const char *text;

  consteval Checked(const char *s) : text(s) {
    const Kind kinds[] = {KindOf<Args>()..., UNSUPPORTED};

    // Pass 1: syntax and field count.
    usize fields = 0;
    usize i = 0;
    while (s[i] != '\0') {
      if (s[i] == '{') {
        Field f{};
        if (!ParseField(s, i, f))
          FormatErrorMalformedField();
        ++fields;
      } else if (s[i] == '}') {
        FormatErrorStrayBrace();
      } else {
        ++i;
      }
    }
    if (fields != sizeof...(Args))
      FormatErrorArgumentCount();

    // Pass 2: each field must suit its argument.
    i = 0;
    usize k = 0;
    while (s[i] != '\0') {
      if (s[i] == '{') {
        Field f{};
        ParseField(s, i, f);
        if (kinds[k] == UNSUPPORTED)
          FormatErrorUnsupportedArgument();
        if ((f.form == CHR || f.form == STR) && f.zeros != 0)
          FormatErrorZeroPadding();
        if (!Allowed(f.form, kinds[k]))
          FormatErrorVerbMismatch();
        ++k;
      } else {
        ++i;
      }
    }
  }
};

// ---------- Output ----------

// Raw bits of an integer, masked to its width so negatives print in hex.
template <typename Arg> constexpr u64 Bits(const Arg &arg) {
  u64 v = static_cast<u64>(arg);
  if constexpr (sizeof(Arg) < sizeof(u64))
    v &= (u64(1) << (sizeof(Arg) * 8)) - 1;
  return v;
}

template <typename Sink>
usize Digits(Sink &sink, u64 value, u64 base, usize Nb_zero_befor) {
  char buf[64];
  usize n = 0;
  do {
    u64 d = value % base;
    buf[n++] = static_cast<char>(d < 10 ? '0' + d : 'a' + (d - 10));
    value /= base;
  } while (value != 0);

  for (usize a = 0, b = n - 1; a < b; ++a, --b) {
    char t = buf[a];
    buf[a] = buf[b];
    buf[b] = t;
  }

  usize pad = Nb_zero_befor > n ? Nb_zero_befor - n : 0;
  for (usize k = 0; k < pad; ++k)
    Emit(sink, "0", 1);
  Emit(sink, buf, n);
  return pad + n;
}

template <typename Sink, typename Arg>
usize Hex(Sink &sink, const Arg &arg, usize Nb_zero_befor) {
  return Digits(sink, Bits(arg), 16, Nb_zero_befor);
}

template <typename Sink, typename Arg>
usize Bin(Sink &sink, const Arg &arg, usize Nb_zero_befor) {
  return Digits(sink, Bits(arg), 2, Nb_zero_befor);
}

template <typename Sink, typename Arg>
usize Dec(Sink &sink, const Arg &arg, usize Nb_zero_befor) {
  usize n = 0;
  u64 mag = static_cast<u64>(arg);
  if constexpr (Arg(-1) < Arg(0)) { // signed type
    if (arg < 0) {
      Emit(sink, "-", 1);
      n = 1;
      mag = 0 - mag;
    }
  }
  return n + Digits(sink, mag, 10, Nb_zero_befor);
}

template <typename Sink, typename Arg>
usize Chr(Sink &sink, const Arg &arg) {
  char c = static_cast<char>(arg);
  Emit(sink, &c, 1);
  return 1;
}

template <typename Sink>
usize Str(Sink &sink, const char *s) {
  usize n = ::string::length(s);
  Emit(sink, s, n);
  return n;
}

template <typename Sink, typename Arg>
usize Ptr(Sink &sink, const Arg &arg, usize Nb_zero_befor) {
  Emit(sink, "0x", 2);
  return 2 + Digits(sink, static_cast<u64>(reinterpret_cast<uptr>(arg)), 16,
                    Nb_zero_befor);
}

template <typename Sink, typename Arg>
usize Dispatch(Sink &sink, Field f, const Arg &arg) {
  if constexpr (KindOf<Arg>() == INTEGER) {
    switch (f.form) {
    case HEX: return Hex(sink, arg, f.zeros);
    case DEC: return Dec(sink, arg, f.zeros);
    case BIN: return Bin(sink, arg, f.zeros);
    default:  return 0;
    }
  } else if constexpr (KindOf<Arg>() == CHARACTER) {
    return Chr(sink, arg);
  } else if constexpr (KindOf<Arg>() == STRING) {
    if (f.form == STR)
      return Str(sink, arg);
    return Ptr(sink, arg, f.zeros);
  } else {
    static_assert(KindOf<Arg>() == POINTER, "fmt: unsupported argument type");
    return Ptr(sink, arg, f.zeros);
  }
}

// Writes literal text up to the next '{' or the end of the string.
template <typename Sink>
usize Literal(Sink &sink, const char *fmt, usize &i) {
  usize run = 0;
  while (fmt[i + run] != '\0' && fmt[i + run] != '{')
    ++run;
  Emit(sink, fmt + i, run);
  i += run;
  return run;
}

// No arguments left: only literal text remains.
template <typename Sink>
usize format(Sink &sink, const char *fmt, usize &i) {
  return Literal(sink, fmt, i);
}

// Each field consumes the next argument.
template <typename Sink, typename Arg, typename... Rest>
usize format(Sink &sink, const char *fmt, usize &i, const Arg &arg,
             const Rest &...rest) {
  usize n = Literal(sink, fmt, i);
  Field f{};
  ParseField(fmt, i, f);
  n += Dispatch(sink, f, arg);
  return n + format(sink, fmt, i, rest...);
}

// Returns the number of characters written.
template <typename Sink, typename... Args>
usize Sprintf(Sink &sink, Checked<typename Id<Args>::type...> fmt,
              Args... args) {
  usize i = 0;
  return format(sink, fmt.text, i, args...);
}

} // namespace FMT

#endif
