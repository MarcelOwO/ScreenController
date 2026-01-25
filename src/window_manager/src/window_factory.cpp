
#include <logging/logger.h>

#include <memory>

#include "../include/window_manager/window_manager.h"
#include "glfw_window.h"

namespace screen_controller {

std::unique_ptr<IWindowManager> WindowFactory::Create(
    const std::shared_ptr<Logger>& logger, void* window_pointer) {
  return std::make_unique(GlfwWindow(&logger, window_pointer))
}

}  // namespace screen_controller
