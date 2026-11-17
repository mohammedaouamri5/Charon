

#include "mm/PMM.hpp"
#include "std/struct/FixedList.hpp"
#include "std/types.h"

namespace PMM {
namespace {
constexpr u32 ONE_MiB = 0x100000;
constexpr u32 FRAM = 0x1000;
extern "C" char __kernel_start;
extern "C" char __kernel_end;

static u32 kernel_start_addr() {
  // round it up the remove one fram so its like we round it down
  u32 start = reinterpret_cast<u32>(&__kernel_start);
  return (start & ~(FRAM - 1)) - 0x1000;
}

static u32 kernel_end_addr() {
  u32 end = reinterpret_cast<u32>(&__kernel_end);
  return (end + FRAM - 1) & ~(FRAM - 1); // round up to the next 4 KiB boundary
}

static UnUsedTable unused_list;
static UsedTable used_list;

} // namespace

bool Init(multiboot_info *p_info) {
  if (!(p_info->flags & (1u << 6))) {
    return false;
  }
  unused_list.Init();
  used_list.Init();

  const u8 *cursor = reinterpret_cast<const u8 *>(p_info->mmap_addr);
  const u8 *table_end = cursor + p_info->mmap_length;

  while (cursor < table_end) {
    const multiboot_mmap_entry *entry =
        reinterpret_cast<const multiboot_mmap_entry *>(cursor);

    if (entry->size == 0)
      break; // malformed entry; stepping by 0 would loop forever

    if (entry->type == 1) {
      u64 start = entry->base_addr;
      u64 end = entry->base_addr + entry->length;
      u64 len = entry->length;

      if (start >= ONE_MiB) {

        if (start >= kernel_end_addr()) {
          if (!unused_list.PushFront({start, len})) {
            return false;
          }
        }
        if (start < kernel_end_addr() && kernel_end_addr() < end) {
          if (!unused_list.PushFront(
                  {kernel_end_addr(), len - (kernel_end_addr() - start)})) {
            return false;
          }
        }
        if (start < kernel_start_addr() && kernel_start_addr() < end) {
          if (!unused_list.PushFront({start, (kernel_start_addr() - start)})) {
            return false;
          }
        }
      }
    }

    cursor += entry->size + 4; // size doesn't count its own field
  }

  return true;
}

void *Alloc(usize _Nb) {
  usize requested_size = _Nb * FRAM;
  for (const usize *pivot = unused_list.GetHead(); pivot;
       pivot = unused_list.GetNode(*pivot)->Next()) {
    const UnUsedTable::Node *node = unused_list.GetNode(*pivot);

    if (node->item.lenght >= requested_size) {
      u64 result = (node->item.start + node->item.lenght - requested_size);
      if (!used_list.PushFront({result, requested_size})) {
        return NULL;
      }
      unused_list.SetAt(*pivot, {node->item.start, node->item.lenght - requested_size});
      return (void*)result;
    }
  }
  return NULL;
}

const UnUsedTable *UnUsedList() { return &unused_list; }
const UsedTable *UsedList() { return &used_list; }

} // namespace PMM
