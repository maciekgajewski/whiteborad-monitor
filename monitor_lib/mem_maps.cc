#include "mem_maps.hh"

#include <fmt/core.h>

#include <fstream>
#include <regex>

namespace Whiteboard {

MemMaps::Mapping MemMaps::parseLine(const std::string &line) {
  static const std::regex RX(
      "^([0-9a-f]+)-([0-9a-f]+) .{4} ([0-9a-f]+) [0-9:]{5} [0-9]+\\s*(.*)$");

  std::smatch match;
  if (!std::regex_match(line, match, RX))
    throw std::runtime_error(fmt::format("Failed to parse line: {}", line));

  MemMaps::Mapping out;
  out.low = std::stoull(match[1], nullptr, 16);
  out.high = std::stoull(match[2], nullptr, 16);
  out.offset = std::stoull(match[3], nullptr, 16);
  out.path = match[4];

  return out;
}

void MemMaps::load(int pid) {

  std::vector<Mapping> mappings;

  std::ifstream f(fmt::format("/proc/{}/maps", pid).c_str());
  std::string line;
  while (std::getline(f, line))
    mappings.push_back(parseLine(line));

  _mappings.swap(mappings);
}

std::uint64_t MemMaps::findAddressByOffset(const std::string &path,
                                           std::uint64_t offset) {

  for (const Mapping &mapping : _mappings) {
    if (mapping.path == path) {
      auto lower = mapping.offset;
      auto upper = mapping.offset + (mapping.high - mapping.low);
      if (offset >= lower && offset < upper) {
        auto mapped_addr = mapping.low + offset - mapping.offset;
        return mapped_addr;
      }
    }
  }

  throw std::runtime_error(
      fmt::format("Address mapping of {}@{:x} not found", path, offset));
}

} // namespace Whiteboard