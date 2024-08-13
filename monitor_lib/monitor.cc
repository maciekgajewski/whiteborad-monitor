#include "monitor.hh"

#include <fmt/core.h>

#include <boost/filesystem.hpp>

#include <algorithm>
#include <cassert>
#include <cstdio>

#include <fcntl.h>
#include <sys/ptrace.h>
#include <sys/user.h>
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

  return Monitor(pid, executable);
}

Monitor::Monitor(int pid, const std::string &executable)
    : _executable(
          boost::filesystem::canonical(boost::filesystem::path(executable))
              .native()),
      _debugInfo(pid, _executable) {

  _childPid = pid;
  _running = true;
  wait();
}

Monitor::~Monitor() = default;

Monitor::StopState Monitor::wait() {
  assert(_running);

  StopState state;

  int wstatus;
  ::waitpid(_childPid, &wstatus, 0);
  if (!WIFSTOPPED(wstatus)) {
    fmt::print("child finished\n", wstatus);
    _running = false;
    state.reason = StopReason::Finished;
  } else {

    // read registers
    ::user_regs_struct regs;
    ::ptrace(PTRACE_GETREGS, _childPid, 0, &regs);
    fmt::print("stopped, RIP=0x{:x}\n", regs.rip);

    auto it =
        std::find_if(_breakpoints.begin(), _breakpoints.end(),
                     [&](auto &bptr) { return bptr.addr == regs.rip - 1; });
    if (it != _breakpoints.end()) {
      fmt::print("breakpoint hit, oid={}\n", it->id);
      state.reason = StopReason::Breakpoint;
      state.breakpoint = it->id;

      disarmBreakpoint(*it);
      _breakpoints.erase(it);
      regs.rip -= 1;
      ::ptrace(PTRACE_SETREGS, _childPid, 0, &regs);
    } else {
      state.reason = StopReason::Other;
    }

    // store registers
    _recentState.registers = Registers::fromLinux(regs);

    // peek into the text
    std::int64_t textLow =
        ::ptrace(PTRACE_PEEKTEXT, _childPid, (void *)regs.rip, nullptr);
    std::int64_t textHigh =
        ::ptrace(PTRACE_PEEKTEXT, _childPid, (void *)(regs.rip + 8), nullptr);
    std::memcpy(_recentState.nextText.data(), &textLow, 8);
    std::memcpy(_recentState.nextText.data() + 8, &textHigh, 8);
  }
  return state;
}

Monitor::StopState Monitor::stepi() {
  assert(_running);
  ::ptrace(PTRACE_SINGLESTEP, _childPid, nullptr, nullptr);
  return wait();
}

Monitor::StopState Monitor::cont() {
  assert(_running);
  ::ptrace(PTRACE_CONT, _childPid, nullptr, nullptr);
  return wait();
}

void Monitor::breakAtFunction(const std::string &fname, breakpoint_id bid) {
  addr_t addr = _debugInfo.findFunction(fname);
  addBreakpoint(addr, bid);
}

void Monitor::addBreakpoint(addr_t addr, breakpoint_id bid) {

  // debug
  fmt::println("Adding bp at address 0x{:x}", addr);

  long data = ::ptrace(PTRACE_PEEKTEXT, _childPid, (void *)addr, nullptr);
  if (data == -1) {
    throw std::runtime_error(fmt::format(
        "Unable to arm a breakpoint (PEEKTEXT): {}", std::strerror(errno)));
  }

  Breakpoint bp;
  bp.addr = addr;
  bp.id = bid;
  bp.originalData = data;

  Word64 w(data);
  w.set8(0, 0xcc);
  fmt::print(
      "setting bp at addr=0x{:x}, original data=0x{:x}, modified=0x{:x}\n",
      bp.addr, bp.originalData, w.get64());

  if (::ptrace(PTRACE_POKETEXT, _childPid, (void *)bp.addr, w.get64())) {
    throw std::runtime_error(fmt::format(
        "Unable to arm a breakpoint (POKETEXT): {}", std::strerror(errno)));
  }

  _breakpoints.push_back(bp);
}

void Monitor::dumpMem(addr_t addr, size_t len) {
  for (int i = 0; i < 10; ++i) {
    auto a = addr + i;
    uint64_t data = ::ptrace(PTRACE_PEEKTEXT, _childPid, (void *)a, nullptr);
    fmt::println("0x{:02x} : 0x{:02x}", a, data & 0xff);
  }
}

void Monitor::disarmBreakpoint(const Breakpoint &bp) {
  if (::ptrace(PTRACE_POKETEXT, _childPid, (void *)bp.addr,
               (void *)(bp.originalData))) {
    throw std::runtime_error(fmt::format(
        "Unable to disarm a breakpoint (POKETEXT): {}", std::strerror(errno)));
  }
}

} // namespace Whiteboard