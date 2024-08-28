
#include "monitor_lib/logging.hh"
#include "monitor_lib/monitor.hh"

#include <fmt/core.h>

#include <thread>

#include <cassert>

using namespace std::literals;

int main(int argc, char **argv) {

  if (argc < 2) {
    fmt::print("Argument required\n");
    return 1;
  }

  Whiteboard::Logging::setLogLevel(Whiteboard::Logging::LogLevel::Trace);

  const char *executable = argv[1];

  Whiteboard::Monitor::Args args = {executable, "1", "2"};
  Whiteboard::Monitor m = Whiteboard::Monitor::runExecutable(executable, args);

  Whiteboard::breakpoint_id mainBreakpointId = 77;
  m.breakAtFunction("main", mainBreakpointId);
  Whiteboard::Word64 mainStackTop;

  auto state = m.cont();
  while (m.isRunning()) {
    fmt::print("process stopped\n");
    if (state.reason == Whiteboard::Monitor::StopReason::Breakpoint) {
      fmt::print("...at breakpoint {}\n", state.breakpoint);
      assert(state.breakpoint ==
             mainBreakpointId); // I'm not expecting any other breakpoint

      // read the stack pointer
      auto registers = m.registers();
      mainStackTop = registers[Whiteboard::Registers::Names::SP];

      auto location = m.currentSourceLocation();
      fmt::println("at loc: {}", location);
    }
    std::this_thread::sleep_for(2s);
    state = m.cont();
  }

  fmt::print("Process {} finished\n", executable);
}
