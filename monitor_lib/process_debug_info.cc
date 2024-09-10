#include "process_debug_info.hh"

#include "logging.hh"

namespace Whiteboard {
ProcessDebugInfo::ProcessDebugInfo(int pid, const std::string &executablePath)
    : _executable(executablePath), _executableDebugInfo(executablePath) {
  _maps.load(pid);
}

addr_t ProcessDebugInfo::findFunction(const std::string &fname) const {
  auto offset = _executableDebugInfo.findFunction(fname);
  return _maps.findAddressByOffset(_executable, offset);
}

std::optional<SourceLocation>
ProcessDebugInfo::findSourceLocation(addr_t addr) const {

  auto [path, offset] = _maps.findFileAndOffsetByAddress(addr);
  if (path != _executable)
    return std::nullopt;

  Logging::trace(
      "ProcessDebugInfo: found source location for addr 0x{:x} in {}@{:x}",
      addr, path, offset);

  return _executableDebugInfo.findSourceLocation(offset);
}

} // namespace Whiteboard