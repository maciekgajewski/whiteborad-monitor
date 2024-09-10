#pragma once

#include <fmt/format.h>

#include <compare>
#include <cstdint>
#include <cstring>

namespace Whiteboard {

// wrapper over 64-bit machine work, providing access to different parts of it
class Word64 {
public:
  Word64() = default;
  Word64(std::uint64_t w64) : _data(w64) {}

  void set8(int index, std::uint8_t v) { std::memcpy(bytes() + index, &v, 1); }

  void set64(std::uint64_t v) { _data = v; }
  std::uint64_t get64() const { return _data; }

  std::uint8_t *bytes() { return reinterpret_cast<std::uint8_t *>(&_data); }

  constexpr auto operator<=>(const Word64 &) const = default;

private:
  std::uint64_t _data = 0;
};

} // namespace Whiteboard

template <> struct fmt::formatter<Whiteboard::Word64> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  auto format(const Whiteboard::Word64 &v, format_context &ctx) const {
    return fmt::format_to(ctx.out(), "0x{:<08x}", v.get64());
  }
};