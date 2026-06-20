#pragma once

#include <enums/log_level.hpp>
#include <memory>
#include <string_view>

namespace screen_controller {

class ILogger {
public:
  virtual ~ILogger() = default;

  virtual void Log(LogLevel level, std::string_view log) = 0;

  void LogInfo(std::string_view log) {
    Log(LogLevel::INFO, log);
  }

  void LogError(std::string_view log) {
    Log(LogLevel::ERROR, log);
  }
};

class LoggerFactory {
public:
  static ILogger& GetInstance();
  static std::unique_ptr<ILogger> Create();
};

};  // namespace screen_controller
