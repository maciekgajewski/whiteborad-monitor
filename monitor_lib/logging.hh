#pragma once

#include <fmt/color.h>
#include <fmt/core.h>

#include <cstdio>

namespace Whiteboard::Logging {

// a brutally simple logging facility

enum class LogLevel { Trace, Debug, Error, None };

void setLogLevel(LogLevel ll);
LogLevel logLevel();

template <typename... Args>
void log(LogLevel ll, fmt::format_string<Args...> f, const Args &...args) {
  if (ll >= logLevel()) {

    fmt::text_style style;
    if (ll == LogLevel::Debug)
      style = fmt::fg(fmt::color::green);
    else if (ll == LogLevel::Error) {
      style = fmt::fg(fmt::color::red);
    } else if (ll == LogLevel::Trace) {
      style = fmt::fg(fmt::color::light_blue);
    }

    fmt::vprint(stderr, style, f, fmt::make_format_args(args...));
    fmt::println(stderr, "");
  }
}

template <typename... Args>
void error(fmt::format_string<Args...> f, const Args &...args) {
  log(LogLevel::Error, f, args...);
}

template <typename... Args>
void debug(fmt::format_string<Args...> f, const Args &...args) {
  log(LogLevel::Debug, f, args...);
}

template <typename... Args>
void trace(fmt::format_string<Args...> f, const Args &...args) {
  log(LogLevel::Trace, f, args...);
}

} // namespace Whiteboard::Logging