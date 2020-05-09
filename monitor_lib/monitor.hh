#pragma once

#include <string>

namespace Whiteboard {

class Monitor {
public:
  Monitor(const std::string &executable);

  void run();

private:
  void runChild();

  std::string _executable;
};

} // namespace Whiteboard