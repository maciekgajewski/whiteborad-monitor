#pragma once

#include "mem_maps.hh"

#include <memory>
#include <string>
#include <vector>

namespace dwarf {
class dwarf;
}

namespace Whiteboard {

namespace Detail {
class BreakpointImpl;
}

union Word {
  std::uint64_t w;
  std::uint8_t b[8];
};

using addr_t = std::uint64_t;

class Breakpoint {
public:
  ~Breakpoint();
  std::unique_ptr<Detail::BreakpointImpl> _impl;
};

class Monitor {
public:
  using Args = std::vector<std::string>;

  Monitor(const Monitor &) = delete;
  Monitor(Monitor &&) = delete;
  ~Monitor();

  static Monitor runExecutable(const std::string &executable, const Args &args);

  bool isRunning() const { return _running; }

  // breakpoints
  Breakpoint functionBreakpoint(const std::string &functionName);

  // execution control
  void stepi();
  void cont();

private:
  friend class Detail::BreakpointImpl;

  Monitor(int pid, const std::string &executable);

  void wait();
  void breakpointRemoved(Detail::BreakpointImpl &bp);
  void armBreakpoint(Detail::BreakpointImpl &bp);
  void disarmBreakpoint(Detail::BreakpointImpl &bp);

  int _childPid = 0;
  std::string _executable;
  bool _running = false;

  std::unique_ptr<dwarf::dwarf> _dwarf;
  std::vector<Detail::BreakpointImpl *> _breakpoints;

  MemMaps _maps;
};

} // namespace Whiteboard