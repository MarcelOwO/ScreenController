#include "glfw_window.hpp"

#include <format>

#include "GLFW/glfw3.h"

namespace screen_controller {

std::expected<std::unique_ptr<GlfwWindow>, std::error_code> GlfwWindow::Create(
    ILogger& logger, const std::function<void()> kOnShutdownRequested) {
  logger.LogInfo("Creating WindowManager");

  if (!kOnShutdownRequested) {
    return std::unexpected(std::make_error_code(std::errc::invalid_argument));
  }

  if (instance_ != nullptr) {
    logger.LogError("Tried to create a second window");
    return std::unexpected(std::make_error_code(std::errc::invalid_argument));
  }

  glfwSetErrorCallback([](int, const char* description) {
    auto* window = GlfwWindow::instance_;
    if (window == nullptr) {
      return;
    }
    const auto kFormatted = std::format("Error in glfw callback: {}", description);
    window->logger_.LogError(kFormatted);
  });

  if (glfwInit() != GLFW_TRUE) {
    logger.LogError("Failed to initialize GLFW");
    return std::unexpected(std::make_error_code(std::errc::connection_refused));
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  auto* raw_window = glfwCreateWindow(1920, 1080, "My Title", glfwGetPrimaryMonitor(), nullptr);

  if (raw_window == nullptr) {
    logger.LogError("Failed to create GLFW window");
    return std::unexpected(std::make_error_code(std::errc::invalid_argument));
  }

  glfwMakeContextCurrent(raw_window);

  glfwSetKeyCallback(raw_window,
                     [](GLFWwindow* window, int key, int scancode, int action, int mods) {
                       if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
                         auto* const kRef = GlfwWindow::instance_;

                         if (kRef == nullptr) {
                           return;
                         }

                         kRef->kOnShutdownRequested();
                       }
                     });

  glfwSwapInterval(1);

  auto window = std::unique_ptr<GlfwWindow>(new GlfwWindow(logger, kOnShutdownRequested));

  window->window_ = std::unique_ptr<GLFWwindow, GlfwWindowDeleter>(raw_window);

  instance_ = window.get();

  return window;
}

GlfwWindow::GlfwWindow(ILogger& logger, const std::function<void()>& on_shutdown_requested)
    : logger_(logger), kOnShutdownRequested(on_shutdown_requested) {}

bool GlfwWindow::should_close() const {
  return glfwWindowShouldClose(window_.get()) != 0;
}

void GlfwWindow::poll_events() {
  return glfwPollEvents();
}

void GlfwWindow::swap_buffers() {
  glfwSwapBuffers(window_.get());
}

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
  logger_.LogInfo("Cleaning up Window");
  instance_ = nullptr;

  glfwTerminate();
  logger_.LogInfo("Cleaned up window");
}

}  // namespace screen_controller
