//
// Created by marce on 4/2/2025.
//

#include <window_manager/window_manager.h>

namespace screen_controller {

WindowManager::WindowManager(std::shared_ptr<Logger> logger)
    : window_(), logger_(logger) {
  logger_->LogInfo("Creating WindowManager");
};

std::expected<void, ErrorEnum> WindowManager::init() {
  logger_->LogInfo("Initializing window manager");

  glfwSetErrorCallback([](int, const char* description) {
    //Fix later lol
    //logger_->LogError(std::string("GLFW error: ") + description);
  });

  if (glfwInit() != GLFW_TRUE) {
    logger_->LogError("Failed to initialize GLFW");
  }

  window_ = glfwCreateWindow(1920, 1080, "My Title", glfwGetPrimaryMonitor(),
                             nullptr);

  if (window_ == nullptr) {
    logger_->LogError("Failed to create GLFW window");
    return std::unexpected(ErrorEnum::ERROR);
  }

  if (window_ == nullptr) {
    logger_->LogError("Feailed to create GLFW window");
    return std::unexpected(ErrorEnum::ERROR);
  }

  glfwMakeContextCurrent(window_);

  return {};
}

bool WindowManager::should_close() const {
  return glfwWindowShouldClose(window_) != 0;
}

void WindowManager::poll_events() const { return glfwPollEvents(); }

void WindowManager::swap_buffers() const { glfwSwapBuffers(window_); }

int WindowManager::get_height() {
  const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
  return mode->height;
}

int WindowManager::get_width() {
  const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
  return mode->width;
}

GLFWglproc (*WindowManager::address_pointer())(const char* procname) {
  return &glfwGetProcAddress;
}

void WindowManager::update(const std::function<void()>& render) const {
  if (window_ == nullptr) {
    return;
  }
  render();
  glfwSwapBuffers(window_);
  glfwSwapInterval(1);
}

WindowManager::~WindowManager() {
  if (window_ != nullptr) {
    glfwDestroyWindow(window_);
  }
  glfwTerminate();
}
}  // namespace screen_controller