#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

namespace Whiteboard {

using offset_t = std::uint64_t;

// Loads DWARF data for ELF executable file
class FileDebugInfo {
public:
  FileDebugInfo(const std::string &path);
  ~FileDebugInfo();

  offset_t findFunction(const std::string &fname) const;

private:
  std::unordered_map<std::string, offset_t> _functions;
};

} // namespace Whiteboard