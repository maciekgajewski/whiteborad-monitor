
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

  int instructions = 0;
  while (m.isRunning()) {
    m.stepi();
    instructions++;
  }

  fmt::print("Process {} finished, executed {} instructions\n", executable,
             instructions);
}
