#pragma once

#include <fmt/format.h>

#include <string>

namespace Whiteboard {

class SourceLocation {
public:
  const std::string file() const { return file_; }
  int line() const { return line_; }

private:
  std::string file_;
  int line_ = 0;
};

} // namespace Whiteboard

template <> struct fmt::formatter<Whiteboard::SourceLocation> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  auto format(const Whiteboard::SourceLocation &sl, format_context &ctx) const {
    return fmt::format_to(ctx.out(), "{}:{}", sl.file(), sl.line());
  }
};