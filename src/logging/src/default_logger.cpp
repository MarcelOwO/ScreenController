#include "default_logger.h"

#include <print>

namespace screen_controller {

DefaultLogger::DefaultLogger(std::string_view name) {}

DefaultLogger::~DefaultLogger() = default;

void DefaultLogger::Log(const LogLevel level, const std::string_view log) {
  switch (level) {
    case LogLevel::INFO: {
      std::println("{}", log);
      break;
    }
    case LogLevel::ERROR: {
      std::println("{}", (log));
      break;
    }
    default: {
      std::println("Unknown log level");
      break;
    }
  }
}

void DefaultLogger::LogInfo(const std::string_view log) {
  Log(LogLevel::INFO, log);
}

void DefaultLogger::LogError(const std::string_view log) {
  Log(LogLevel::ERROR, log);
}

}  // namespace screen_controller
