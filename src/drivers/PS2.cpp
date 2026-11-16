#include "drivers/PS2.hpp"
#include "std/struct/bits.hpp"
#include "std/io.hpp"
#include "std/types.h"

namespace PS2 {

enum Set {
  SetOne = 0b00000000,
  SetTwo = 0b01000000,
};

namespace {

constexpr u16 kDataPort = 0x60;
constexpr u16 kStatusPort = 0x64;
constexpr u16 kCommandPort = 0x64;

inline u8 ReadStatus() { return io::inb(kStatusPort); }
inline u8 ReadData() { return io::inb(kDataPort); }
inline void WriteCommand(u8 command) { io::outb(kCommandPort, command); }
inline void WriteData(u8 data) { io::outb(kDataPort, data); }

bool IsOutputFull() { return GET_BIT(ReadStatus(), 0); }
bool IsInputFull() { return GET_BIT(ReadStatus(), 1); }
bool MouseSended() { return GET_BIT(ReadStatus(), 5); }

bool CommandExpects(u8 command, u8 expected) {
  io::wait();
  WriteCommand(command);
  io::wait();
  return ReadData() == expected;
}

} // namespace

void EnableKeyBord() {
  io::wait();
  WriteCommand(0xAE);
}

void DisabledKeybord() {
  io::wait();
  WriteCommand(0xAD);
}

void EnableMouse() {
  io::wait();
  WriteCommand(0xA8);
}

void DisabledMouse() {
  io::wait();
  WriteCommand(0xA7);
}

u8 GetConfig() {
  io::wait();
  WriteCommand(0x20);
  io::wait();
  return ReadData();
}

u8 GetStatus() {
  return ReadStatus();
}

void SetConfig(u8 config) {
  io::wait();
  WriteCommand(0x60);
  io::wait();
  WriteData(config);
}

void useSet(Set set) { SetConfig(GetConfig() | set); }

bool SelfTest() { return CommandExpects(0xAA, 0x55); }
bool SelfTestKeybord() { return CommandExpects(0xAB, 0x00); }
bool SelfTestMouse() { return CommandExpects(0xA9, 0x00); }


Err Pull(u8 &data, u8 &status, bool forMouse) {
  u8 s = ReadStatus();

  if (!(s & 0x01))
    return Err::ErrOutputBufferEmpty;
  data = ReadData();

  status = s;
  if ((s & 0x20) != forMouse) {
    return Err::ErrRowngDevice; // consumed, but meant for the other device
  }
  return Err::OK;
}

u8 Init() {
  u8 err = 0;

  DisabledKeybord();
  DisabledMouse();
  while (IsOutputFull())
    ReadData(); // flush

  u8 cfg = GetConfig();
  cfg &= ~(1 << 0 | 1 << 1 | 1 << 6); // IRQs off, translation off
  SetConfig(cfg);

  if (!SelfTest())
    return ErrSelfTestFail;
  SetConfig(cfg); // self-test may reset config

  EnableMouse();
  bool dual = !GET_BIT(GetConfig(), 5);
  if (dual)
    DisabledMouse();

  if (!SelfTestKeybord())
    err |= ErrSelfTestKeybordFail;
  if (dual && !SelfTestMouse())
    err |= ErrSelfTestMouseFail;
  if (err)
    return err;

  EnableKeyBord();
  if (dual)
    EnableMouse();

  cfg = GetConfig();
  cfg |= (1 << 0) | (dual << 1); // enable IRQ1 (+ IRQ12)
  SetConfig(cfg);

  return 0;
}
} // namespace PS2
