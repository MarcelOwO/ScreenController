//
// Created by marce on 4/2/2025.
//

#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <logging/logger.h>
#include <models/error_enum.h>

#include <expected>
#include <functional>
#include <memory>

namespace screen_controller {
class WindowManager {
 public:
  explicit WindowManager(std::shared_ptr<Logger> logger);
  ~WindowManager();
  WindowManager(const WindowManager&) = delete;
  WindowManager& operator=(const WindowManager&) = delete;
  WindowManager(WindowManager&&) = delete;
  WindowManager& operator=(WindowManager&&) = delete;

  [[nodiscard]] std::expected<void, common::ErrorEnum> init();

  void update(const std::function<void()>& render) const;

  [[nodiscard]] bool should_close() const;
  void poll_events() const;

  void swap_buffers() const;

  static int get_height();
  static int get_width();

  static GLFWglproc (*address_pointer())(const char* procname);

 private:
  GLFWwindow* window_;
  std::shared_ptr<Logger> logger_;
};
}  // namespace screen_controller

#endif  // WINDOW_MANAGER_H
