#ifndef DEFAULT_LOGGER_H
#define DEFAULT_LOGGER_H

#include "../include/logging/logger.h"
namespace screen_controller {

class DefaultLogger : public ILogger {
 public:
  explicit DefaultLogger(std::string_view name);
  ~DefaultLogger();

  void Log(LogLevel level, std::string_view log);
  void LogInfo(std::string_view log);

  void LogError(std::string_view log);
};

}  // namespace screen_controller

#endif  // DEFAULT_LOGGER_H_
