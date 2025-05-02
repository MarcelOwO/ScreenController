//
// Created by marce on 4/2/2025.
//

#include "window_manager.h"

#include <iostream>
namespace screen_controller {
WindowManager::WindowManager() = default;

bool WindowManager::init() {
  glfwSetErrorCallback([](int, const char* description) {
    std::cout << "Error: " << description << "\n";
  });

  if (!glfwInit()) {
    return false;
  }

  window_ = glfwCreateWindow(1920, 1080, "My Title", glfwGetPrimaryMonitor(),
                             nullptr);

  if (!window_) {
    return false;
  }

  glfwMakeContextCurrent(window_);

  return true;
}

bool WindowManager::should_close() const {
  return glfwWindowShouldClose(window_);
}

void WindowManager::poll_events() { return glfwPollEvents(); }

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
  if (!window_) {
    return;
  }

  render();

  glfwSwapBuffers(window_);

  glfwSwapInterval(1);
}

WindowManager::~WindowManager() {
  if (window_) {
    glfwDestroyWindow(window_);
  }
  glfwTerminate();
}

}  // namespace screen_controller
