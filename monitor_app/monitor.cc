
#include "monitor_lib/monitor.hh"

#include <fmt/core.h>

#include <thread>

using namespace std::literals;

int main(int argc, char **argv) {

  if (argc < 2) {
    fmt::print("Argument required");
    return 1;
  }

  const char *executable = argv[1];

  monitor::monitor m(executable);
  m.run();
}
