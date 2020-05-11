#include "file_debug_info.hh"

#include <dwarf/dwarf++.hh>
#include <elf/elf++.hh>

#include <fmt/core.h>

#include <cstring>

#include <fcntl.h>
#include <unistd.h>

namespace Whiteboard {

FileDebugInfo::FileDebugInfo(const std::string &path) {

  int fd = ::open(path.c_str(), O_RDONLY);
  if (fd < 0) {
    throw std::runtime_error(
        fmt::format("Unable to open executable: {}", std::strerror(errno)));
  }

  auto loader = elf::create_mmap_loader(fd);
  elf::elf ef = elf::elf(loader);
  _dwarf = std::make_unique<dwarf::dwarf>(dwarf::elf::create_loader(ef));
}

FileDebugInfo::~FileDebugInfo() = default;

/*
static void dump(const dwarf::die &node, std::string indent) {
  fmt::print("{}tag={}\n", indent, dwarf::to_string(node.tag));

  for (const auto &[at, value] : node.attributes()) {
    fmt::print("{}* {} = {}\n", indent, dwarf::to_string(at),
               dwarf::to_string(value));
  }

  indent += "  ";
  for (const dwarf::die &child : node) {
    dump(child, indent);
  }
}
*/

offset_t FileDebugInfo::findFunction(const std::string &fname) const {

  for (const dwarf::compilation_unit &cu : _dwarf->compilation_units()) {
    for (const dwarf::die &node : cu.root()) {
      if (node.tag == dwarf::DW_TAG::subprogram &&
          node.has(dwarf::DW_AT::name)) {
        if (fname == dwarf::at_name(node)) {
          dwarf::taddr addr = dwarf::at_low_pc(node);
          return addr;
        }
      }
    }
  }

  throw std::runtime_error(fmt::format("Function '{}' not found", fname));
}

} // namespace Whiteboard