#include <logging/logger.h>

#include <ng-log/logging.h>

namespace screen_controller {

Logger::Logger(std::string_view name) {
  nglog::InitializeLogging(name.data());
  FLAGS_logtostderr = true;
}

Logger::~Logger() = default;

void Logger::Log(const LogLevel level, const std::string_view log) {
  switch (level) {
    case LogLevel::INFO: {
      LOG(INFO) << log;
      break;
    }
    case LogLevel::ERROR: {
      LOG(ERROR) << log;
      break;
    }
    default: {
      LOG(ERROR) << "Unknown LogLevel";
      break;
    }
  }
}

void Logger::LogInfo(const std::string_view log) { Log(LogLevel::INFO, log); }
void Logger::LogError(const std::string_view log) { Log(LogLevel::ERROR, log); }

}  // namespace screen_controller
