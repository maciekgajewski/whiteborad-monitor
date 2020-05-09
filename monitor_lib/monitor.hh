#pragma once

#include <string>
#include <vector>
namespace Whiteboard {

class Monitor {
public:
  using Args = std::vector<std::string>;

  Monitor(const Monitor &) = delete;

  static Monitor runExecutable(const std::string &executable, const Args &args);

  bool isRunning() const { return _running; }
  void stepi();

private:
  Monitor(int pid);

  int _childPid = 0;
  bool _running = false;
};

} // namespace Whiteboard