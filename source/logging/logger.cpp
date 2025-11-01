#include <logging/logger.h>
#include <print>
#include <ng-log/logging.h>

namespace screen_controller {

Logger::Logger(std::string_view name) {
}

Logger::~Logger() = default;

void Logger::Log(const LogLevel level, const std::string_view log) {
  switch (level) {
    case LogLevel::INFO: {
      std::println("{}",log);
      break;
    }
    case LogLevel::ERROR: {
      std::println("{}",(log));
      break;
    }
    default: {
      std::println("Unknown log level");
      break;
    }
  }
}

void Logger::LogInfo(const std::string_view log) { Log(LogLevel::INFO, log); }

void Logger::LogError(const std::string_view log) { Log(LogLevel::ERROR, log); }

}  // namespace screen_controller
