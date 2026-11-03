#ifndef STD_STRUCT_STACK_HPP
#define STD_STRUCT_STACK_HPP

#include "std/types.h"

template <typename T, usize N>
struct Stack {
  T data[N];
  usize size = 0;

  bool Empty() const { return size == 0; }
  bool Full() const { return size == N; }

  bool Push(const T &v) {
    if (Full()) return false;
    data[size++] = v;
    return true;
  }
  bool Pop(T &out) {
    if (Empty()) return false;
    out = data[--size];
    return true;
  }
  void Clear() { size = 0; }
};
#endif
