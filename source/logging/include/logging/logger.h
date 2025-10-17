
#ifndef LOGGER_H
#define LOGGER_H

#include <string_view>

namespace screen_controller {

enum class LogLevel {
  INFO,
  ERROR,
};

class Logger {
 public:
  explicit Logger(std::string_view name);
  ~Logger();

   void Log(LogLevel level, std::string_view log);
   void LogInfo(std::string_view log);

   void LogError(std::string_view log);
};

};  // namespace screen_controller
#endif
