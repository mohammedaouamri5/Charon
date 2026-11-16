#ifndef __BITS_HPP__
#define __BITS_HPP__

#define GET_BIT(buff, index) (((buff) >> (index)) & 1)

#include "std/types.h"

template <usize N> struct Bitset {
  static constexpr usize WordBits = 32;
  static constexpr usize Words = (N + WordBits - 1) / WordBits;

  u32 Data[Words];

  void ClearAll() {
    for (usize i = 0; i < Words; i++)
      Data[i] = 0;
  }

  void SetAll() {
    for (usize i = 0; i < Words; i++)
      Data[i] = 0xFFFFFFFFu;
  }

  inline bool Test(usize i) const {
    if (i >= N)
      return false;
    return ((Data[i / WordBits] >> (i % WordBits)) & 1u) != 0;
  }


  void Set(usize i) {
    if (i >= N)
      return;
    Data[i / WordBits] |= (1u << (i % WordBits));
  }

  void Clear(usize i) {
    if (i >= N)
      return;
    Data[i / WordBits] &= ~(1u << (i % WordBits));
  }

  void Flip(usize i) {
    if (i >= N)
      return;
    Data[i / WordBits] ^= (1u << (i % WordBits));
  }

  usize Size() const { return N; }
};

#endif
