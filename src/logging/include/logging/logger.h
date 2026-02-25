#ifndef LOGGER_H
#define LOGGER_H

#include <memory>
#include <string_view>

namespace screen_controller {

enum class LogLevel {
  INFO,
  ERROR,
};

class ILogger {
 public:
  virtual ~ILogger() = default;

  virtual void Log(LogLevel level, std::string_view log) = 0;

  void LogInfo(std::string_view log) { Log(LogLevel::INFO, log); }
  void LogError(std::string_view log) { Log(LogLevel::ERROR, log); }
};

class LoggerFactory {
 public:
  virtual ILogger& GetInstance();
  virtual std::shared_ptr<ILogger> Create();
};

};  // namespace screen_controller
#endif
