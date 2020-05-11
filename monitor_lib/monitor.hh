#pragma once

#include "process_debug_info.hh"

#include <memory>
#include <string>
#include <vector>

namespace Whiteboard {

union Word {
  std::uint64_t w;
  std::uint8_t b[8];
};

union Dword {
  Word words[2];
  std::uint8_t b[16];
};

using addr_t = std::uint64_t;
using breakpoint_id = std::uint64_t;

struct Registers {
  addr_t rip; // instruction pointer
  // TODO add others as needed
};

class Monitor {
public:
  using Args = std::vector<std::string>;

  enum class StopReason { Breakpoint, Finished, Other };

  struct StopState {
    StopReason reason;
    breakpoint_id breakpoint;
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

  int _childPid = 0;
  std::string _executable;
  bool _running = false;

  std::vector<Breakpoint> _breakpoints;
  ProcessDebugInfo _debugInfo;

  struct {
    Registers registers;
    Dword nextText;
  } _recentState;
};

} // namespace Whiteboard