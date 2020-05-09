
#include "run.hh"

#include <fmt/core.h>

int main(int argc, char **argv) {

  if (argc < 2) {
    std::cout << "argument required" << std::endl;
    return 1;
  }

  const char *executable = argv[1];

  whiteboard::run(executable);
}
