#pragma once

#include <string>
#include <vector>
namespace Whiteboard {

class Monitor {
public:
  using Args = std::vector<std::string>;

  Monitor(const std::string &executable, const Args &args);

  void run();

private:
  void runChild();

  std::string _executable;
  Args _args;
};

} // namespace Whiteboard