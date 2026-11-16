
#ifndef STD_STRUCT_RING_HPP
#define STD_STRUCT_RING_HPP

#include "std/types.h"

template <typename T, usize N> struct Ring {
  T data[N];
  usize start = 0;
  usize end = 0;

  bool Empty() const { return start == end; }
  // Capacity for Full() is N-1; Push silently drops the oldest entry.
  bool Full() const { return (end + 1) % N == start; }

  void Push(const T &v) {
    data[end] = v;
    end = (end + 1) % N;
    if (end == start)
      start = (start + 1) % N;
  }

  // Pops the newest element. Same shape as Stack::Pop so callers can use both.
  bool PopNew(T &out) {
    if (Empty())
      return false;
    end = (end + N - 1) % N;
    out = data[end];
    return true;
  }

  bool PopOld(T &out) {
    if (Empty())
      return false;
    out = data[start];
    start = (start + 1) % N;
    return true;
  }

  T *Peek() const {
    if (Empty())
      return nullptr;
    return &data[start];
  }
  void Clear() { start = end = 0; }
};

// Single-producer single-consumer FIFO. Unlike Ring, Push fails when full
// instead of dropping the oldest entry. Unsigned cursor wraparound makes the
// fullness check (push - pop >= N) correct without a wasted slot. Elements
// are default-constructed upfront and moved in/out with assignment, so T
// must be default-constructible and move-assignable.
template <typename T, usize N> class SPSCRing {
  T data_[N];
  usize push_cursor_ = 0;
  usize pop_cursor_ = 0;

  bool Full(usize push_cursor, usize pop_cursor) const {
    return push_cursor - pop_cursor >= N;
  }

  bool Empty(usize push_cursor, usize pop_cursor) const {
    return push_cursor == pop_cursor;
  }

public:
  SPSCRing() = default;

  SPSCRing(const SPSCRing &) = delete;
  SPSCRing &operator=(const SPSCRing &) = delete;

  bool Push(const T &v) {
    usize push_cursor = push_cursor_;
    usize pop_cursor = pop_cursor_;

    if (Full(push_cursor, pop_cursor))
      return false;

    data_[push_cursor % N] = v;
    push_cursor_ = push_cursor + 1;
    return true;
  }

  bool Push(T &&v) {
    usize push_cursor = push_cursor_;
    usize pop_cursor = pop_cursor_;

    if (Full(push_cursor, pop_cursor))
      return false;

    data_[push_cursor % N] = static_cast<T &&>(v);
    push_cursor_ = push_cursor + 1;
    return true;
  }

  bool Pop(T &out) {
    usize push_cursor = push_cursor_;
    usize pop_cursor = pop_cursor_;

    if (Empty(push_cursor, pop_cursor))
      return false;

    T &slot = data_[pop_cursor % N];
    out = static_cast<T &&>(slot);
    slot = T();

    pop_cursor_ = pop_cursor + 1;
    return true;
  }
};

#endif
