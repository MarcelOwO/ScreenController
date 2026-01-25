#include <logging/logger.h>

#include <memory>

#include "../include/window_manager/window_manager.h"
#include "glfw_window.h"

namespace screen_controller {

class GlfwWindow : public IWindowManager {
 public:
  explicit GlfwWindow(const std::shared_ptr<Logger>& logger);
  ~GlfwWindow();

  GlfwWindow(const GlfwWindow&) = delete;
  GlfwWindow& operator=(const GlfwWindow&) = delete;
  GlfwWindow(GlfwWindow&&) = delete;
  GlfwWindow& operator=(GlfwWindow&&) = delete;

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
