
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
  bool Pop(T &out) {
    if (Empty())
      return false;
    end = (end + N - 1) % N;
    out = data[end];
    return true;
  }

  T *Peek() const {
    if (Empty())
      return nullptr;
    return &data[start];
  }
  void Clear() { start = end = 0; }
};

#endif
