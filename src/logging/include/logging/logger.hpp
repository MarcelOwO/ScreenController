#pragma once

#include <enums/log_level.hpp>

#include <format>
#include <memory>
#include <string_view>

namespace screen_controller {

class ILogger {
public:
  virtual ~ILogger() = default;

  virtual void Log(LogLevel level, std::string_view message) = 0;

  void LogTrace(std::string_view msg) { Log(LogLevel::TRACE, msg); }
  void LogDebug(std::string_view msg) { Log(LogLevel::DEBUG, msg); }
  void LogInfo(std::string_view msg) { Log(LogLevel::INFO, msg); }
  void LogWarn(std::string_view msg) { Log(LogLevel::WARN, msg); }
  void LogError(std::string_view msg) { Log(LogLevel::ERROR, msg); }

  template <typename... Args>
  void LogFmt(LogLevel level, std::format_string<Args...> fmt, Args&&... args) {
    Log(level, std::format(fmt, std::forward<Args>(args)...));
  }
};

class LoggerFactory {
public:
  static std::unique_ptr<ILogger> Create();
};

}  // namespace screen_controller
