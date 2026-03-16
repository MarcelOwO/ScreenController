#include "glfw_window.h"

#include <format>

#include "GLFW/glfw3.h"

namespace screen_controller {

std::expected<std::unique_ptr<GlfwWindow>, std::error_code> GlfwWindow::create(
    ILogger& logger, const std::function<void()> onShutdownRequested) {
  logger.LogInfo("Creating WindowManager");

  if (!onShutdownRequested) {
    return std::unexpected(std::make_error_code(std::errc::invalid_argument));
  }

  if (instance != nullptr) {
    logger.LogError("Tried to create a second window");
    return std::unexpected(std::make_error_code(std::errc::invalid_argument));
  }

  glfwSetErrorCallback([](int, const char* description) {
    auto window = GlfwWindow::instance;
    if (window == nullptr) {
      return;
    }
    const auto formatted =
        std::format("Error in glfw callback: {}", description);
    window->_logger.LogError(formatted);
  });

  if (glfwInit() != GLFW_TRUE) {
    logger.LogError("Failed to initialize GLFW");
    return std::unexpected(std::make_error_code(std::errc::connection_refused));
  }

  auto raw_window = glfwCreateWindow(1920, 1080, "My Title",
                                     glfwGetPrimaryMonitor(), nullptr);

  if (raw_window == nullptr) {
    logger.LogError("Failed to create GLFW window");
    return std::unexpected(std::make_error_code(std::errc::invalid_argument));
  }

  glfwMakeContextCurrent(raw_window);

  glfwSetKeyCallback(raw_window, [](GLFWwindow* window, int key, int scancode,
                                    int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
      const auto ref = GlfwWindow::instance;

      if (ref == nullptr) {
        return;
      }

      ref->onShutdownRequested_();
    }
  });

  glfwSwapInterval(1);

  auto window =
      std::unique_ptr<GlfwWindow>(new GlfwWindow(logger, onShutdownRequested));

  window->window_ = std::unique_ptr<GLFWwindow, GlfwWindowDeleter>(raw_window);

  instance = window.get();

  return window;
}

GlfwWindow::GlfwWindow(ILogger& logger,
                       const std::function<void()>& onShutdownRequested)
    : window_(), _logger(logger), onShutdownRequested_(onShutdownRequested) {}

bool GlfwWindow::should_close() const {
  return glfwWindowShouldClose(window_.get()) != 0;
}

void GlfwWindow::poll_events() { return glfwPollEvents(); }

void GlfwWindow::swap_buffers() { glfwSwapBuffers(window_.get()); }

int GlfwWindow::get_height() const {
  const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
  return mode->height;
}

int GlfwWindow::get_width() const {
  const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
  return mode->width;
}

IWindowManager::ProcLoader GlfwWindow::get_proc_address() const {
  return reinterpret_cast<IWindowManager::ProcLoader>(glfwGetProcAddress);
}

void GlfwWindow::update(const std::function<void()>& render) {
  if (window_ == nullptr) {
    return;
  }

  render();

  glfwSwapBuffers(window_.get());
}

GlfwWindow::~GlfwWindow() {
  instance = nullptr;

  glfwTerminate();
}

}  // namespace screen_controller
