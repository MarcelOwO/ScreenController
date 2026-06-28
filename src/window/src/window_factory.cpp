#include <helper/unwrap.hpp>
#include <logging/logger.hpp>
#include <memory>
#include <window_manager/window_manager.hpp>

#include "glfw_window.hpp"

namespace screen_controller {

std::unique_ptr<IWindowManager> WindowFactory::Create(ILogger& logger,
                                                      std::function<void()> on_shutdown_requested) {
  return Unwrap(GlfwWindow::Create(logger, std::move(on_shutdown_requested)),
                "Failed to create window manager");
}

}  // namespace screen_controller
