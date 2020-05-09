#include "monitor.hh"

#include <fmt/core.h>

#include <cerrno>
#include <cstring>

#include <sys/ptrace.h>
#include <unistd.h>

namespace Whiteboard {

Monitor::Monitor(const std::string &executable) : _executable(executable) {
  fmt::print("creating monitor on {}\n", executable);
}

void Monitor::run() {

  fmt::print("running\n");

  int pid = ::fork();
  if (pid == 0)
    runChild();

  // attach to child
  // TODO
}

void Monitor::runChild() {

  ::execl(_executable.c_str(), _executable.c_str(), nullptr);
  throw std::runtime_error(
      fmt::format("Failed to execute target: {}", std::strerror(errno)));
}

} // namespace Whiteboard