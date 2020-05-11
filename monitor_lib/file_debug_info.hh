#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace dwarf {
class dwarf;
}

namespace Whiteboard {

using offset_t = std::uint64_t;

// Loads DWARF data for ELF executable file
class FileDebugInfo {
public:
  FileDebugInfo(const std::string &path);
  ~FileDebugInfo();

  offset_t findFunction(const std::string &fname) const;

private:
  std::unique_ptr<dwarf::dwarf> _dwarf;
};

} // namespace Whiteboard