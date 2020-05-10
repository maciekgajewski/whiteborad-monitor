#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace Whiteboard {

// Loads and keeps data from /proc/PID/maps
class MemMaps {
public:
  void load(int pid);

  // returns addres, in process space, of a byte mapped from file at offset
  std::uint64_t findAddressByOffset(const std::string &path,
                                    std::uint64_t offset);

private:
  struct Mapping {
    std::uint64_t low, high, offset;
    std::string path;
  };

  static Mapping parseLine(const std::string &line);

  std::vector<Mapping> _mappings;
};

} // namespace Whiteboard