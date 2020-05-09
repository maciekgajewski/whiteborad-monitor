
#include "monitor_lib/monitor.hh"

#include <fmt/core.h>

#include <thread>

using namespace std::literals;

int main(int argc, char **argv) {

  if (argc < 2) {
    fmt::print("Argument required\n");
    return 1;
  }

  const char *executable = argv[1];

  Whiteboard::Monitor::Args args = {executable, "1", "2"};
  Whiteboard::Monitor m = Whiteboard::Monitor::runExecutable(executable, args);

  auto breakpoint = m.functionBreakpoint("main");
  m.cont();
  while (m.isRunning()) {
    fmt::print("process stopped\n");
    std::this_thread::sleep_for(2s);
    m.cont();
  }

  fmt::print("Process {} finished\n", executable);
}
