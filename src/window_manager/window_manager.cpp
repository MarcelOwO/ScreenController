//
// Created by marce on 4/2/2025.
//

#include "window_manager.h"

#include <ng-log/logging.h>

#include <iostream>

namespace screen_controller {
WindowManager::WindowManager() : window_() {
};

bool WindowManager::init() {
  LOG(INFO) << "Creating window manager";

  (void)glfwSetErrorCallback([](int, const char* description) {
    PLOG(ERROR) << "GLFW error: " << description;
  });

  CHECK(glfwInit() != 0) << "Failed to initialize GLFW";

  window_ = glfwCreateWindow(1920, 1080, "My Title", glfwGetPrimaryMonitor(),
                             nullptr);

  CHECK(window_ != nullptr) << "Failed to create GLFW window";

  glfwMakeContextCurrent(window_);

  return true;
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

GLFWglproc (* WindowManager::address_pointer())(const char* procname) {
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
} // namespace screen_controller