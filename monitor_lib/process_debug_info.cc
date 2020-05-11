#include "process_debug_info.hh"

namespace Whiteboard {
ProcessDebugInfo::ProcessDebugInfo(int pid, const std::string &executablePath)
    : _executable(executablePath), _executableDebugInfo(executablePath) {
  _maps.load(pid);
}

addr_t ProcessDebugInfo::findFunction(const std::string &fname) {
  auto offset = _executableDebugInfo.findFunction(fname);
  return _maps.findAddressByOffset(_executable, offset);
}

} // namespace Whiteboard