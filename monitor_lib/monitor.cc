#include "monitor.hh"

#include <fmt/core.h>

namespace Whiteboard {

Monitor::Monitor(const std::string &executable) : _executable(executable) {
  fmt::print("creating monitor on {}\n", executable);
}

void Monitor::run() { fmt::print("running\n"); }

} // namespace Whiteboard