#include "monitor.hh"

#include <fmt/core.h>

#include <algorithm>
#include <cerrno>
#include <cstring>

#include <sys/ptrace.h>
#include <unistd.h>

namespace Whiteboard {

Monitor::Monitor(const std::string &executable, const Args &args)
    : _executable(executable), _args(args) {
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

  std::vector<char *> argv;
  argv.reserve(_args.size() + 1);

  for (auto &arg : _args)
    argv.push_back(arg.data());
  argv.push_back(nullptr);

  ::execv(_executable.c_str(), argv.data());
  throw std::runtime_error(
      fmt::format("Failed to execute target: {}", std::strerror(errno)));
}

} // namespace Whiteboard