#pragma once

#include "file_debug_info.hh"
#include "mem_maps.hh"
#include "source_location.hh"

#include <optional>
#include <string>

namespace Whiteboard {

using addr_t = std::uint64_t;

// Keeps debug info for a running process.
// Allows for translating symbols <-> process-space addresses
class ProcessDebugInfo {
public:
  ProcessDebugInfo(int pid, const std::string &executablePath);

  addr_t findFunction(const std::string &fname) const;
  std::optional<SourceLocation> findSourceLocation(addr_t addr) const;

private:
  std::string _executable;
  FileDebugInfo _executableDebugInfo;
  MemMaps _maps;
};

} // namespace Whiteboard