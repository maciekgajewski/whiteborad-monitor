#pragma once

#include <libdwarf/dwarf.h>
#include <libdwarf/libdwarf.h>

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
  void process_dwarf_die(Dwarf_Die &die, Dwarf_Error &error, int in_level);
  void process_dwarf_cu(Dwarf_Die &cu_die, const char *die_name,
                        Dwarf_Error &error);
  void walk_dwarf_die(Dwarf_Debug dbg, Dwarf_Die in_die, int is_info,
                      int in_level, Dwarf_Error &error);
  std::unordered_map<std::string, offset_t> _functions;
};

} // namespace Whiteboard