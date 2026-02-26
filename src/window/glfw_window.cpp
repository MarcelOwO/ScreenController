#include "glfw_window.h"

namespace screen_controller {

GlfwWindow::GlfwWindow(ILogger& logger) : window_(), _logger(logger) {
  _logger.LogInfo("Creating WindowManager");

  glfwSetErrorCallback([](int, const char* description) {});

  if (glfwInit() != GLFW_TRUE) {
    _logger.LogError("Failed to initialize GLFW");
    throw std::runtime_error("Failed to initialize GLFW");
  }

  window_ = glfwCreateWindow(1920, 1080, "My Title", glfwGetPrimaryMonitor(),
                             nullptr);

  if (window_ == nullptr) {
    _logger.LogError("Failed to create GLFW window");
    throw std::runtime_error("Failed to create GLFW window");
  }

  glfwMakeContextCurrent(window_);
};

bool GlfwWindow::should_close() const {
  return glfwWindowShouldClose(window_) != 0;
}

void GlfwWindow::poll_events() { return glfwPollEvents(); }

void GlfwWindow::swap_buffers() { glfwSwapBuffers(window_); }

int GlfwWindow::get_height() const {
  const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
  return mode->height;
}

int GlfwWindow::get_width() const {
  const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
  return mode->width;
}

void GlfwWindow::update(const std::function<void()>& render) {
  if (window_ == nullptr) {
    return;
  }

  render();

  glfwSwapBuffers(window_);
  glfwSwapInterval(1);
}

GlfwWindow::~GlfwWindow() {
  if (window_ != nullptr) {
    glfwDestroyWindow(window_);
  }
  glfwTerminate();
}
}  // namespace screen_controller
