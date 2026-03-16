#include "../include/logging/logger.h"
#include "default_logger.h"

namespace screen_controller {

std::unique_ptr<ILogger> LoggerFactory::Create() {
  auto logger = DefaultLogger::Create();

  if (!logger) {
    throw std::runtime_error("failed to create logger");
  }

  return std::move(logger.value());
};

}  // namespace screen_controller
