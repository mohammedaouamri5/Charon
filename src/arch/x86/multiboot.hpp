#include "std/types.h"

struct [[gnu::packed]] multiboot_mmap_entry {
  u32 size; // size of the rest of this entry (20 here), not counting this field
  u64 base_addr; // start address of the region
  u64 length;    // size of the region in bytes
  u32 type; // 1 = usable RAM, 2 = reserved, 3 = ACPI reclaimable, 4 = ACPI NVS,
            // 5 = bad RAM
};
struct [[gnu::packed]] multiboot_info {
  u32 flags; // always valid; bitmask saying which fields below are valid

  u32 mem_lower; // flags bit 0: KiB of RAM below 1 MiB
  u32 mem_upper; // flags bit 0: KiB of RAM above 1 MiB

  u32 boot_device; // flags bit 1: BIOS drive the kernel was loaded from
  u32 cmdline;     // flags bit 2: physical address of the kernel command line
  u32 mods_count;  // flags bit 3: number of boot modules
  u32 mods_addr;   // flags bit 3: physical address of the module descriptors

  u32 syms[4]; // flags bit 4 (a.out) or bit 5 (ELF): symbol table info

  u32 mmap_length; // flags bit 6: total size of the memory map in bytes
  multiboot_mmap_entry
      *mmap_addr; // flags bit 6: physical address of the first map entry

  u32 drives_length; // flags bit 7: total size of the drive info in bytes
  u32 drives_addr;   // flags bit 7: physical address of the drive info

  u32 config_table; // flags bit 8: physical address of the BIOS config table
  u32 boot_loader_name; // flags bit 9: physical address of the bootloader name
                        // string
};

