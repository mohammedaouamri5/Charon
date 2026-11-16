
#include "std/types.h"

namespace PS2 {


enum Err {
  OK = 0,
  ErrSelfTestFail = 1 << 0,
  ErrSelfTestMouseFail = 1 << 1,
  ErrSelfTestKeybordFail = 1 << 2,
  ErrRowngDevice = 1 << 3,
  ErrOutputBufferEmpty = 1 << 4,

};



Err Pull(u8 &data , u8 &status, bool forMouse = false);
u8 Init();
u8 GetConfig();
u8 GetStatus();
} // namespace PS2
