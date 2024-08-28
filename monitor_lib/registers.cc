#include "registers.hh"

#include <sys/user.h>

namespace Whiteboard {

Registers Registers::fromLinux(const ::user_regs_struct &regs) {

  Registers out;
  out[SP].set64(regs.rsp);
  out[IP].set64(regs.rip);
  // TODO set others
  return out;
}

} // namespace Whiteboard