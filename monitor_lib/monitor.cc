#include "monitor.hh"

#include <dwarf/dwarf++.hh>
#include <elf/elf++.hh>

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

namespace Detail {
struct BreakpointImpl {
  BreakpointImpl(std::uint64_t a, Monitor *m) : _addr(a), _monitor(m) {}
  ~BreakpointImpl() {
    if (_monitor)
      _monitor->breakpointRemoved(*this);
  }
  BreakpointImpl(BreakpointImpl &&) = delete;
  BreakpointImpl(const BreakpointImpl &) = delete;

  std::uint64_t _addr;
  std::uint64_t _originalData;
  Monitor *_monitor;
  bool _armed = false;
};
} // namespace Detail

Breakpoint::~Breakpoint() = default;

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

Monitor::Monitor(int pid, const std::string &executable) {

  namespace bfs = boost::filesystem;
  _executable = bfs::canonical(bfs::path(executable)).native();

  int fd = ::open(_executable.c_str(), O_RDONLY);
  if (fd < 0) {
    throw std::runtime_error(
        fmt::format("Unable to open executable: {}", std::strerror(errno)));
  }

  auto loader = elf::create_mmap_loader(fd);
  elf::elf ef = elf::elf(loader);
  _dwarf = std::make_unique<dwarf::dwarf>(dwarf::elf::create_loader(ef));

  _childPid = pid;
  _running = true;
  wait();
  _maps.load(_childPid);
}

Monitor::~Monitor() {
  for (Detail::BreakpointImpl *bp : _breakpoints)
    bp->_monitor = nullptr;
}

void Monitor::wait() {
  assert(_running);

  int wstatus;
  ::waitpid(_childPid, &wstatus, 0);
  if (!WIFSTOPPED(wstatus)) {
    fmt::print("child finished\n", wstatus);
    _running = false;
  } else {
    struct user_regs_struct regs;
    ::ptrace(PTRACE_GETREGS, _childPid, 0, &regs);
    fmt::print("stopped, RIP={:x}\n", regs.rip);
    auto it =
        std::find_if(_breakpoints.begin(), _breakpoints.end(),
                     [&](auto &bptr) { return bptr->_addr == regs.rip - 1; });
    if (it != _breakpoints.end()) {
      fmt::print("breakpoint hit\n");
      disarmBreakpoint(**it);
      // move back and execute the instrumention at breakpoint again
      regs.rip -= 1;
      ::ptrace(PTRACE_SETREGS, _childPid, 0, &regs);
    }
  }
}

void Monitor::stepi() {
  assert(_running);
  ::ptrace(PTRACE_SINGLESTEP, _childPid, nullptr, nullptr);
  wait();
}

void Monitor::cont() {
  assert(_running);
  ::ptrace(PTRACE_CONT, _childPid, nullptr, nullptr);
  wait();
}

/*
static void dump(const dwarf::die &node, std::string indent) {
  fmt::print("{}tag={}\n", indent, dwarf::to_string(node.tag));

  for (const auto &[at, value] : node.attributes()) {
    fmt::print("{}* {} = {}\n", indent, dwarf::to_string(at),
               dwarf::to_string(value));
  }

  indent += "  ";
  for (const dwarf::die &child : node) {
    dump(child, indent);
  }
}
*/

static std::uint64_t findFunctionByName(const dwarf::dwarf &dwarf,
                                        const std::string &name) {
  for (const dwarf::compilation_unit &cu : dwarf.compilation_units()) {
    for (const dwarf::die &node : cu.root()) {
      if (node.tag == dwarf::DW_TAG::subprogram &&
          node.has(dwarf::DW_AT::name)) {
        if (name == dwarf::at_name(node)) {
          dwarf::taddr addr = dwarf::at_low_pc(node);
          return addr;
        }
      }
    }
  }
  return 0;
}

void Monitor::breakpointRemoved(Detail::BreakpointImpl &bp) {

  auto it = std::find(_breakpoints.begin(), _breakpoints.end(), &bp);
  assert(it != _breakpoints.end());
  _breakpoints.erase(it);
  if (bp._armed && _running)
    disarmBreakpoint(bp);
}

Breakpoint Monitor::functionBreakpoint(const std::string &functionName) {

  // use dwarf to find function name
  auto offset = findFunctionByName(*_dwarf, functionName);
  if (offset) {
    std::uint64_t addr = _maps.findAddressByOffset(_executable, offset);
    auto bi = new Detail::BreakpointImpl{addr, this};
    armBreakpoint(*bi);
    _breakpoints.push_back(bi);
    return Breakpoint{std::unique_ptr<Detail::BreakpointImpl>(bi)};
  } else
    throw std::runtime_error(
        fmt::format("Unable to find function: {}", functionName));
}

void Monitor::armBreakpoint(Detail::BreakpointImpl &bp) {
  if (bp._armed)
    return;

  long data = ::ptrace(PTRACE_PEEKTEXT, _childPid, (void *)bp._addr, nullptr);
  if (data == -1) {
    throw std::runtime_error(fmt::format(
        "Unable to arm a breakpoint (PEEKTEXT): {}", std::strerror(errno)));
  }
  bp._originalData = data;

  Word w;
  w.w = data;
  w.b[0] = 0xcc;

  if (::ptrace(PTRACE_POKETEXT, _childPid, (void *)bp._addr, (void *)(w.w))) {
    throw std::runtime_error(fmt::format(
        "Unable to arm a breakpoint (POKETEXT): {}", std::strerror(errno)));
  }
}

void Monitor::disarmBreakpoint(Detail::BreakpointImpl &bp) {
  if (::ptrace(PTRACE_POKETEXT, _childPid, (void *)bp._addr,
               (void *)(bp._originalData))) {
    throw std::runtime_error(fmt::format(
        "Unable to disarm a breakpoint (POKETEXT): {}", std::strerror(errno)));
  }
  bp._armed = false;
}

} // namespace Whiteboard