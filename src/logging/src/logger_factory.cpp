#include "../include/logging/logger.h"
#include "default_logger.h"

namespace screen_controller {

std::shared_ptr<ILogger> LoggerFactory::Create() {
  auto logger = DefaultLogger::create();

  if (!logger) {
    return nullptr;
  }

  return std::move(logger.value());
};

}  // namespace screen_controller
