
#include <logging/logger.h>
#include <window_manager/window_manager.h>

#include <memory>

#include "glfw_window.h"

namespace screen_controller {

std::unique_ptr<IWindowManager> WindowFactory::Create(
    ILogger& logger, const std::function<void()> kOnShutdownRequested) {
  auto window = GlfwWindow::create(logger, kOnShutdownRequested);

  if (!window) {
    throw std::runtime_error("Failed to create Window manager");
  }

  return std::move(window.value());
}
}  // namespace screen_controller
