#include "monitor.hh"

#include <fmt/core.h>

#include <algorithm>
#include <cassert>
#include <cstdio>

#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>

namespace Whiteboard {

Monitor Monitor::runExecutable(const std::string &executable,
                               const Args &args) {

  fmt::print("running {}\n", executable);

  int pid = ::fork();
  if (pid == 0) {

    if (::ptrace(PTRACE_TRACEME, 0, nullptr, nullptr)) {
      std::perror("ptrace failed");
      std::abort();
    }

    std::vector<const char *> argv;
    argv.reserve(args.size() + 1);

    for (auto &arg : args)
      argv.push_back(arg.c_str());
    argv.push_back(nullptr);

    ::execv(executable.c_str(), const_cast<char **>(argv.data()));
    std::perror("Failed ot execute target");
    std::abort();
  }

  return Monitor(pid);
}

Monitor::Monitor(int pid) {

  _childPid = pid;

  int wstatus;
  ::waitpid(_childPid, &wstatus, 0);
  if (WIFSTOPPED(wstatus))
    _running = true;
}

void Monitor::stepi() {
  assert(_running);

  ::ptrace(PTRACE_SINGLESTEP, _childPid, nullptr, nullptr);

  int wstatus;
  ::waitpid(_childPid, &wstatus, 0);
  if (!WIFSTOPPED(wstatus))
    _running = false;
}

} // namespace Whiteboard