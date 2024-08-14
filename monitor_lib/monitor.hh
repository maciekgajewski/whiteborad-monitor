#pragma once

#include "process_debug_info.hh"
#include "registers.hh"
#include "word.hh"

#include <memory>
#include <string>
#include <vector>

namespace Whiteboard {

using addr_t = std::uint64_t;
using breakpoint_id = std::uint64_t;

class Monitor {
public:
  using Args = std::vector<std::string>;

  enum class StopReason { Breakpoint, Finished, Other };

  struct StopState {
    StopReason reason;
    breakpoint_id breakpoint = 0;
  };

  Monitor(const Monitor &) = delete;
  Monitor(Monitor &&) = delete;
  ~Monitor();

  static Monitor runExecutable(const std::string &executable, const Args &args);

  bool isRunning() const { return _running; }

  // breakpoints
  void breakAtFunction(const std::string &functionName, breakpoint_id bid);

  // execution control
  StopState stepi();
  StopState cont();

  const Registers &registers() const { return _recentState.registers; }

private:
  struct Breakpoint {
    addr_t addr;
    std::uint64_t originalData;
    breakpoint_id id;
  };

  Monitor(int pid, const std::string &executable);

  StopState wait();
  void addBreakpoint(addr_t addr, breakpoint_id bid);
  void disarmBreakpoint(const Breakpoint &bp);

  // prints memory at address
  void dumpMem(addr_t addr, size_t len);

  int _childPid = 0;
  std::string _executable;
  bool _running = false;

  std::vector<Breakpoint> _breakpoints;
  ProcessDebugInfo _debugInfo;

  struct {
    Registers registers;
  } _recentState;
};

} // namespace Whiteboard