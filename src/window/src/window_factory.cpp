
#include <logging/logger.hpp>
#include <window_manager/window_manager.hpp>

#include <memory>

#include "glfw_window.hpp"

namespace screen_controller {

std::unique_ptr<IWindowManager> WindowFactory::Create(
    ILogger& logger, const std::function<void()> kOnShutdownRequested) {
  auto window = GlfwWindow::Create(logger, kOnShutdownRequested);

  if (!window) {
    throw std::runtime_error("Failed to create Window manager");
  }

  return std::move(window.value());
}
}  // namespace screen_controller
