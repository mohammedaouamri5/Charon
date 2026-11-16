#include "kernel.hpp"
#include "KeyLayout.hpp"
#include "arch/x86/GDT.hpp"
#include "arch/x86/IDT.hpp"
#include "drivers/Keyboard.hpp"
#include "drivers/PIC.hpp"
#include "drivers/PIT.hpp"
#include "drivers/PS2.hpp"
#include "drivers/VGA.hpp"
#include "drivers/VGA_themes.hpp"
#include "mm/PMM.hpp"
#include "multiboot.hpp"
#include "std/struct/FixedList.hpp"
#include "std/struct/bits.hpp"
#include "std/types.h"
#include "terminal.hpp"

static void Prompt(Terminal &terminal) { terminal.WriteString("charon> "); }

/* exact compare of the first len characters against a known word */
static bool IsWord(const char *line, u32 len, const char *word) {
  while (len > 0 && *word != '\0') {
    if (*line++ != *word++)
      return false;
    len--;
  }
  return len == 0 && *word == '\0';
}

// trashy shell
static void RunCommand(Terminal &terminal, const char *line, u32 len) {
  if (len == 0)
    return;
  if (IsWord(line, len, "help")) {
    terminal.WriteString("[ INFO ] commands: help, clear, echo <text>\n");
    return;
  }
  if (IsWord(line, len, "clear")) {
    terminal.Flush();
    return;
  }
  if (IsWord(line, len, "PMM-info")) {
    const PMM::UnUsedTable *unused_frams = PMM::UnUsedList();
    const Bitset<UNUSED_MMAP_SIZE> *unused_mask = unused_frams->GetUsed();

    terminal.WriteString("------------------UNUSED-------------------\n");
    terminal.WriteString("the bit map is\n");

    for (usize i = 0; i < UNUSED_MMAP_SIZE; i++) {
      if (!unused_mask->Test(i))
        continue;

      const PMM::UnUsedTable::Node *node = unused_frams->GetNode(i);
      if (!node)
        continue;

      terminal.WriteString("the index : ");
      terminal.WriteU64(i);
      terminal.PutChar('\n');
      terminal.WriteString("the start  : ");
      terminal.WriteHex(node->item.start);
      terminal.PutChar('\n');
      terminal.WriteString("the length : ");
      terminal.WriteU64(node->item.lenght);
      terminal.PutChar('\n');
    }



    terminal.WriteString("-------------------USED--------------------\n");
    const PMM::UsedTable *used_frams = PMM::UsedList();
    const Bitset<USED_MMAP_SIZE> *used_mask = used_frams->GetUsed();

    terminal.WriteString("the bit map is\n");
    for (usize i = 0; i < UNUSED_MMAP_SIZE; i++) {
      if (!used_mask->Test(i))
        continue;
      const PMM::UsedTable::Node *node = used_frams->GetNode(i);
      if (!node)
        continue;

      terminal.WriteString("the index : ");
      terminal.WriteU64(i);
      terminal.PutChar('\n');
      terminal.WriteString("the start  : ");
      terminal.WriteHex(node->item.start);
      terminal.PutChar('\n');
      terminal.WriteString("the length : ");
      terminal.WriteU64(node->item.lenght);
      terminal.PutChar('\n');
    }

    return;
  }

  if (IsWord(line, len, "PMM-alloc-400")) {
    PMM::Alloc(400);
    return;
  }

  if (line[0] == 'e' && line[1] == 'c' && line[2] == 'h' && line[3] == 'o') {
    const char *text = line + 4;
    u32 rest = len - 4;
    while (rest > 0 && *text == ' ') { // skip spaces after the word
      text++;
      rest--;
    }
    terminal.Write(text, rest);
    terminal.PutChar('\n');
    return;
  }

  terminal.WriteString("[ ERR  ] unknown command\n");
}

extern "C" {
void kernel_main(u32 magic, multiboot_info *p_info) {
  /* Initialize terminal interface */

  GDT_ptr gdt;
  gdt.init();
  IDT_ptr idt;
  idt.init();
  // PMM
  PMM::Init(p_info);

  PIC::Remap();
  PIT::Init();
  u8 ps2err = PS2::Init();
  Keybord::Init(); // unmasks IRQ1 itself, like PIT does for IRQ0

  Terminal terminal; // ~6.4 KB on the stack after the shadow removal
  terminal.Init();
  VGA::DAC::set_palette(0, VGA::DAC::THEMES::monokai_palette, 16);

  terminal.WriteString("[ INFO ] Hello, World!\n"
                       "[ INFO ] This is my first bootable kernel\n"
                       "[ INFO ] It called Charon o_o\n");
  terminal.WriteString("[ INFO ] Type 'help' for commands\n");
  if (ps2err) {
    terminal.WriteString("[ ERR  ] PS/2 init failed, keyboard may not work\n");
  }

  static char line[80];
  u32 len = 0;
  bool caps = false;

  Prompt(terminal);
  for (;;) {
    Keybord::KeyEvent ev;
    if (!Keybord::Poll(ev)) {
      asm volatile("hlt");
      continue;
    }

    /* modifier handling */
    if (ev.code == 0x58 && ev.pressed) { // Caps Lock is a toggle
      caps = !caps;
      continue;
    }
    if (!ev.pressed) // ignore releases
      continue;

    bool shift = Keybord::IsDown(0x12); // Left Shift held

    if (ev.code == 0x66) { // Backspace
      if (len > 0) {
        len--;
        terminal.PutChar('\b');
        terminal.PutChar(' ');
        terminal.PutChar('\b');
      }
      continue;
    }
    if (ev.code == 0x5A) { // Enter
      terminal.PutChar('\n');
      RunCommand(terminal, line, len);
      len = 0;
      Prompt(terminal);
      continue;
    }
    if (ev.ext) // arrows etc.: ignored for now
      continue;

    char c = KeyLayout::DecodeKey(ev.code, shift, caps);
    if (c && len < sizeof(line) - 1) {
      line[len++] = c;
      terminal.PutChar(c);
    }
  }
}
}
