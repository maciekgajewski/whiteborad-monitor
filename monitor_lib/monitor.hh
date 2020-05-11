#pragma once

#include "mem_maps.hh"

#include <memory>
#include <string>
#include <vector>

namespace dwarf {
class dwarf;
}

namespace Whiteboard {

union Word {
  std::uint64_t w;
  std::uint8_t b[8];
};

using addr_t = std::uint64_t;
using breakpoint_id = std::uint64_t;

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

  std::unique_ptr<dwarf::dwarf> _dwarf;
  std::vector<Breakpoint> _breakpoints;

  MemMaps _maps;
};

} // namespace Whiteboard