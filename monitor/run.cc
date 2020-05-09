#include "run.hh"

#include <fmt/core.h>

namespace whiteboard {

void run(const std::string &executable) {
  fmt::print("runnning {}\n", executable);
}
} // namespace whiteboard