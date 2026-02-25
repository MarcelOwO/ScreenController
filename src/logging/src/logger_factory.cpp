#include "../include/logging/logger.h"
#include "default_logger.h"

namespace screen_controller {

std::shared_ptr<ILogger> LoggerFactory::Create() {
  return std::make_shared<DefaultLogger>();
};

}  // namespace screen_controller
