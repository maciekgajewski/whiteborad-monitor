#include "logging.hh"

namespace Whiteboard::Logging {

static LogLevel g_currentLogLevel = LogLevel::None;

void setLogLevel(LogLevel ll) { g_currentLogLevel = ll; }
LogLevel logLevel() { return g_currentLogLevel; }

}