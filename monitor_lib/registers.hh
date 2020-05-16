#pragma once

#include "word.hh"

#include <array>

struct user_regs_struct; // defined in <sys/user.h>

namespace Whiteboard {

// x86_64 registers
class Registers {
public:
  enum Names {
    A,
    C,
    D,
    B,
    SI,
    DI,
    SP,
    BP,
    R8,
    R9,
    R10,
    R11,
    R12,
    R13,
    R14,
    R15,
    R16,

    IP,

    NUM_REGISTERS
  };

  static Registers fromLinux(const ::user_regs_struct &regs);

  Word64 &operator[](int idx) { return _registers[idx]; }
  const Word64 &operator[](int idx) const { return _registers[idx]; }

private:
  std::array<Word64, NUM_REGISTERS> _registers;
};

}; // namespace Whiteboard