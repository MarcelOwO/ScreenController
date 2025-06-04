//
// Created by marce on 4/2/2025.
//

#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <functional>

namespace screen_controller {
class WindowManager {
 public:
  WindowManager();
  ~WindowManager();

  bool init();
  void update(const std::function<void()>& render) const;

  bool should_close() const;
  void poll_events() const;

  void swap_buffers() const;

  int get_height() const;
  int get_width() const;

  static GLFWglproc (*address_pointer())(const char* procname);

 private:
  GLFWwindow* window_;
};
}  // namespace screen_controller

#endif  // WINDOW_MANAGER_H
