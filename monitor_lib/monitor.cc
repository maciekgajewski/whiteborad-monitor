#include "monitor.hh"

#include <fmt/core.h>

namespace monitor {

monitor::monitor(const std::string &executable) {
  fmt::print("creating monitor on {}\n", executable);
}

void monitor::run() { fmt::print("running\n"); }

} // namespace monitor