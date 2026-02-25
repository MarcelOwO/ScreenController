
#include <logging/logger.h>
#include <window_manager/window_manager.h>

#include <memory>

#include "glfw_window.h"

namespace screen_controller {

std::unique_ptr<IWindowManager> WindowFactory::Create(ILogger& logger) {
  try {
    auto window = std::make_unique<GlfwWindow>(logger);
    return window;

  } catch (const std::exception& e) {
    logger.LogError("Failed to create window");
    return nullptr;
  }
}

}  // namespace screen_controller
