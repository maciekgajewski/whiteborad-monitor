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

  auto maybeMapping = _maps.tryFindFileAndOffsetByAddress(addr);
  if (!maybeMapping)
    return std::nullopt;

  auto [path, offset] = *maybeMapping;
  if (path != _executable)
    return std::nullopt;

  return _executableDebugInfo.findSourceLocation(offset);
}

} // namespace Whiteboard