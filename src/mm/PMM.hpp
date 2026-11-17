#ifndef PMM_HPP
#define PMM_HPP

#include "arch/x86/multiboot.hpp"
#include "std/struct/FixedList.hpp"

namespace PMM {

#define USED_MMAP_SIZE 50
#define UNUSED_MMAP_SIZE 20

struct Region {
  u64 start;
  u64 lenght;
};

typedef FixedList<Region, USED_MMAP_SIZE> UsedTable;
typedef FixedList<Region, UNUSED_MMAP_SIZE> UnUsedTable;

bool Init(multiboot_info *p_info);
void *Alloc(usize _Nb);
const UnUsedTable *UnUsedList();
const UsedTable *UsedList();
} // namespace PMM

#endif // !PMM_HPP
