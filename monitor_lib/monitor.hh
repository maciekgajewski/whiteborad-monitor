#pragma once

#include <string>
#include <vector>
namespace Whiteboard {

struct Breakpoint {};

class Monitor {
public:
  using Args = std::vector<std::string>;


  Monitor(const Monitor &) = delete;

  static Monitor runExecutable(const std::string &executable, const Args &args);

  bool isRunning() const { return _running; }

  // breakpoints
  Breakpoint functionBreakpoint(const std::string &functionName);

  // execution control
  void stepi();
  void cont();

private:
  Monitor(int pid);

  void wait();

  int _childPid = 0;
  bool _running = false;
};

} // namespace Whiteboard