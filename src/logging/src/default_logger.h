#ifndef DEFAULT_LOGGER_H
#define DEFAULT_LOGGER_H

#include "../include/logging/logger.h"

namespace screen_controller {

class DefaultLogger : public ILogger {
 public:
  explicit DefaultLogger();
  ~DefaultLogger();
  void Log(LogLevel level, std::string_view log);
};

}  // namespace screen_controller

#endif  // DEFAULT_LOGGER_H_
