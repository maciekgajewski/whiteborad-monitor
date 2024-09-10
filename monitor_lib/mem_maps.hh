#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace Whiteboard {

// Loads and keeps data from /proc/PID/maps
class MemMaps {
public:
  void load(int pid);

  // returns address, in process space, of a byte mapped from file at offset
  std::uint64_t findAddressByOffset(const std::string &path,
                                    std::uint64_t offset) const;

  // returns offset and file, based on process-space address, throws if not
  // found
  std::tuple<std::string, uint64_t>
  findFileAndOffsetByAddress(std::uint64_t addr) const;

  // Returns offset and file, based on process-space address, returns nullopt
  // if not found
  std::optional<std::tuple<std::string, uint64_t>>
  tryFindFileAndOffsetByAddress(std::uint64_t addr) const noexcept;

private:
  struct Mapping {
    std::uint64_t low, high, offset;
    std::string path;
  };

  static Mapping parseLine(const std::string &line);

  std::vector<Mapping> _mappings;
};

} // namespace Whiteboard