
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
  std::optional<Whiteboard::SourceLocation> lastLocation;
  unsigned instructions = 0;

  while (m.isRunning()) {
    fmt::print("process stopped\n");
    if (state.reason == Whiteboard::Monitor::StopReason::Breakpoint) {
      fmt::print("...at breakpoint {}\n", state.breakpoint);
      assert(state.breakpoint ==
             mainBreakpointId); // I'm not expecting any other breakpoint

      // read the stack pointer
      auto registers = m.registers();
      mainStackTop = registers[Whiteboard::Registers::Names::SP];
      fmt::println("main stack top: {}", mainStackTop);

      // iterate over the instructions, until leaving stack
      while (true) {

        registers = m.registers();
        // have we left stack?
        auto sp = registers[Whiteboard::Registers::Names::SP];
        Whiteboard::Logging::trace("instruction #{}, SP={}, main stack top={}",
                                   instructions, sp, mainStackTop);
        if (sp > mainStackTop) {
          fmt::println("EVENT main completed, SP={}", sp);
          break;
        }

        // see if source location changed
        auto maybeLocation = m.currentSourceLocation();
        if (maybeLocation) {
          Whiteboard::Logging::trace("source loc: {}", *maybeLocation);
          if (!lastLocation || *lastLocation != *maybeLocation) {
            fmt::println("EVENT: source loc: {}", *maybeLocation);
            lastLocation = *maybeLocation;
          }
        }

        ++instructions;
        auto stopState = m.stepi();
        if (stopState.reason == Whiteboard::Monitor::StopReason::Finished) {
          Whiteboard::Logging::debug("Process finished without leaving main");
          break;
        }
      }
    }
    if (m.isRunning()) {
      state = m.cont();
    }
  }

  fmt::println("Process {} finished. Processed {} instructions", executable,
               instructions);
}
