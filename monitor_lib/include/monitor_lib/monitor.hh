#pragma once

#include <string>

namespace monitor {

class monitor {
public:
  monitor(const std::string &executable);

  void run();
};

}