#pragma once

#include "source_location.hh"

#include <libdwarf/dwarf.h>
#include <libdwarf/libdwarf.h>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace Whiteboard {

using offset_t = std::uint64_t;

// Loads DWARF data for ELF executable file
class FileDebugInfo {
public:
  FileDebugInfo(const std::string &path);
  ~FileDebugInfo();

  offset_t findFunction(const std::string &fname) const;
  std::optional<SourceLocation> findSourceLocation(offset_t offset) const;

private:
  struct LineInfo {
    // offset range: [start, end)
    offset_t start = 0;
    offset_t end = 0;

    SourceLocation location;
  };

  void processDwarfDIE(Dwarf_Die &die, Dwarf_Error &error, int in_level);
  void processDwarfCU(Dwarf_Die &cu_die, const char *die_name,
                      Dwarf_Error &error);
  void walkDwarfDIE(Dwarf_Debug dbg, Dwarf_Die in_die, int is_info,
                    int in_level, Dwarf_Error &error);

  std::vector<std::filesystem::path> getDirs(Dwarf_Line_Context line_context,
                                             Dwarf_Error &error) const;

  std::vector<std::filesystem::path>
  getFiles(Dwarf_Line_Context line_context, Dwarf_Error &error,
           const std::vector<std::filesystem::path> &dirs) const;

  std::unordered_map<std::string, offset_t> _functions;
  std::vector<LineInfo> _lines;
};

} // namespace Whiteboard