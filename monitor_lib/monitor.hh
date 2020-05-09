#pragma once

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
  bool _running = false;

  std::unique_ptr<dwarf::dwarf> _dwarf;
  std::vector<Detail::BreakpointImpl *> _breakpoints;
};

} // namespace Whiteboard