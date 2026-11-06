# Charon

Charon is a hobby operating system for 32-bit x86, written in modern C++20 with
a little GAS assembly. It boots with GRUB via the Multiboot standard and
currently features a working VGA text-mode driver, a scrollable terminal with
history, hardware cursors and full custom color themes, plus a basic GDT.

> Charon — the ferryman of the dead in Greek mythology, carrying souls across
> the Styx. A fitting name for software that brings a machine from firmware
> into a new world.

## What it can do so far

- **Multiboot boot** (`src/boot.s`) — proper multiboot header, 16 KiB kernel
  stack, clean entry into C++.
- **GDT** (`src/GDT.cpp`, `src/GDT.s`) — 5-entry flat-segment table (kernel +
  user code/data) loaded via `lgdt` after `cli; jmp` segment reload.
- **VGA text-mode driver** (`src/VGA.*`) —
  - 80×25 cell buffer with rectangular ops: `put`, `fill_rect`, `move_rect`
    (memmove semantics), `snap_shot`, `put_buffer`
  - hardware CRTC cursor: shape (underline / block), position, show / hide
  - attribute controller: blink vs. bright-background bit
  - DAC palette programming (6-bit RGB) so the 16 "textmode colors" can be
    anything you want
- **Terminal** (`src/terminal.*`) — line-based terminal with scrollback:
  a `Ring` for history (older lines) and a bounded `Stack` for newer lines,
  activate / deactivate the screen, and a cursor that stays in sync.
- **Color themes** (`src/VGA_themes.hpp`) — standard VGA, amber phosphor,
  green phosphor, cyberpunk and monokai palettes, all validated at compile
  time with `static_assert` (6-bit DAC range checks). The kernel boots in
  **monokai**.
- **Freestanding std** (`src/std/`) — fixed-width types and limits
  (`std/types.h`), a mini string library, and container primitives
  (`Ring<T, N>`, `Stack<T, N>`). No compiler runtime beyond `-lgcc`.

## Building

Charon needs an i686 ELF cross-compiler (`i686-elf-g++`, `i686-elf-as`),
CMake ≥ 3.20, `grub-mkrescue`, and QEMU.

```sh
# configure + build the kernel image
cmake -B build
cmake --build build     # -> build/charon.bin

# bootable ISO through GRUB
cmake --build build --target iso      # -> build/charon.iso

# run it
cmake --build build --target run        # raw kernel via QEMU -kernel
cmake --build build --target run-iso    # full GRUB boot, like a real USB stick
```

Other CMake targets: `debug` (QEMU + interrupt logs into `qemu.log`) and
`check-multiboot` (verify the header is valid).

## Project layout

```
src/
├── boot.s            Multiboot header, 16 KiB stack, entry _start
├── kernel.cpp        kernel_main: GDT + terminal init, hello world
├── GDT.{hpp,cpp,s}   Flat GDT (kernel + user segments) and lgdt flush
├── VGA.{hpp,cpp}     Text-mode driver: cells, rects, cursor, mode, DAC
├── VGA_themes.hpp    Theme palettes (standard/amber/green/cyberpunk/monokai)
├── terminal.{hpp,cpp}  Scrollback terminal built on the VGA driver
├── utile.hpp
└── std/
    ├── types.h, string.{h,cpp}  Freestanding fixed-width types + strings
    └── struct/                 Ring & Stack container primitives
link.ld              Kernel loaded at 2 MiB, page-aligned sections
iso/boot/grub.cfg    GRUB menu entry
CMakeLists.txt       Freestanding build (C++20, no exceptions / RTTI), ISO/run targets
```

## Roadmap

> The plan, in rough order — each layer generally has to exist before the
> next one becomes pleasant. Checked items are things that already work.

**Foundations** ✅ *mostly here*
- [x] Multiboot header, kernel stack, clean entry into C++ (`boot.s`)
- [x] Linker script: kernel loaded at 2 MiB, page-aligned sections
- [x] GDT with kernel + user segments and assembly flush routine
- [x] VGA text-mode driver (cells, rectangles, scroll, hardware cursor, blink mode)
- [x] DAC palette programming + 5 compile-time-validated color themes
- [x] Scrollback terminal built on `Ring`/`Stack` containers
- [x] Freestanding std: fixed-width types, string helpers, containers
- [ ] IDT + exception handlers (labels + register dumps for all 32 CPU faults)
- [ ] PIC remapping & timer (PIT) — real interrupts instead of `cli; hlt`
- [ ] Keyboard driver (PS/2) + a simple shell so the terminal becomes interactive
- [ ] Serial (COM1) port — logging that survives a triple fault

**Memory**
- [ ] Physical memory manager (from the multiboot memory map — the bootloader already provides it)
- [ ] Page allocator / buddy or bitmap allocator
- [ ] Virtual memory: paging + identity map, higher-half kernel
- [ ] Kernel heap with a `kmalloc`/`kfree` and malloc-friendly `new`

**Multitasking**
- [ ] TSS + privilege rings actually used (GDT user segments are ready)
- [ ] Kernel threads / context switching, then scheduling (round-robin first)
- [ ] User mode + syscalls
- [ ] Short-term: SMP / APIC

**Userspace & beyond**
- [ ] Real filesystem: ramfs → FAT32 or ext2 on a disk image
- [ ] VFS layer so more filesystems can coexist
- [ ] ELF program loader
- [ ] Graphics: move from text to VBE linear framebuffer + a font renderer
- [ ] ACPI power management (`hlt` with interrupts, poweroff)
- [ ] Long term: port to Multiboot 2 / x86_64

---

Licensed under the [MIT License](LICENSE) © 2026 Mohammed Aouamri.
