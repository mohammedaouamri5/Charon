#ifndef FIXED_LIST_HPP
#define FIXED_LIST_HPP

#include "std/struct/bits.hpp"
#include "std/types.h"

template <class T, usize N> struct FixedList {
  static constexpr usize Nil = N;

  struct Node {
    T item;
    usize next;
    const usize *Next() const {
      if (next == Nil)
        return NULL;
      else
        return &next;
    }
  };

  Node data[N];

  FixedList() : head(Nil) { used.ClearAll(); }

  void Init() {
    head = Nil;
    used.ClearAll();
  }

  const Node *PushFront(const T &item) {
    usize slot = FindFree();
    if (slot == Nil)
      return nullptr;

    data[slot].item = item;
    data[slot].next = head;
    used.Set(slot);
    head = slot;

    return &data[slot];
  }

  bool PopFront(T &out) {
    if (head == Nil)
      return false;

    usize slot = head;
    out = data[slot].item;
    head = data[slot].next;
    used.Clear(slot);

    return true;
  }

  bool IsEmpty() const { return head == Nil; }

  const Node *GetNode(usize slot) const {
    if (slot >= N || !used.Test(slot))
      return NULL;
    return &data[slot];
  }

  const Node *GetNode() const {
    if (head >= N || !used.Test(head))
      return NULL;
    return &data[head];
  }
  const usize *GetHead() const {
    if (head == Nil) {
      return NULL;
    }
    return &head;
  }

  bool SetAt(usize slot, const T &item) {
    if (slot >= N || !used.Test(slot))
      return false;

    data[slot].item = item;
    return true;
  }

  const Node *PushAfter(usize slot, const T &item) {
    if (slot >= N || !used.Test(slot)) {
      return NULL;
    }
    usize new_ = FindFree();
    if (new_ == Nil) {
      return NULL;
    }
    data[new_].item = item;
    data[new_].next = data[slot].next;
    data[slot].next = new_;
    used.Set(new_);
    return &data[new_];
  }

  bool PopAfter(usize slot, T &out) {
    if (slot >= N || !used.Test(slot))
      return false;

    usize victim = data[slot].next;
    if (victim == Nil)
      return false;

    out = data[victim].item;
    data[slot].next = data[victim].next;
    used.Clear(victim);

    return true;
  }
  bool PopAfter(usize slot) {
    if (slot >= N || !used.Test(slot))
      return false;

    usize victim = data[slot].next;
    if (victim == Nil)
      return false;

    data[slot].next = data[victim].next;
    used.Clear(victim);

    return true;
  }

  const Bitset<N> *GetUsed() const { return &used; }

private:
  usize head = Nil;
  Bitset<N> used;
  usize FindFree() const {
    for (usize i = 0; i < N; i++) {
      if (!used.Test(i))
        return i;
    }
    return Nil;
  }
};
#endif
