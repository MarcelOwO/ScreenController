
#include <logging/logger.h>
#include <window_manager/window_manager.h>

#include <expected>

#include "GLFW/glfw3.h"

namespace screen_controller {

class GlfwWindow : public IWindowManager {
 public:
  inline static GlfwWindow* instance = nullptr;

  static std::expected<std::unique_ptr<GlfwWindow>, std::error_code> create(
      ILogger& logger, const std::function<void()> onShutdownRequested);

  ~GlfwWindow();

  GlfwWindow(const GlfwWindow&) = delete;
  GlfwWindow& operator=(const GlfwWindow&) = delete;
  GlfwWindow(GlfwWindow&&) = delete;
  GlfwWindow& operator=(GlfwWindow&&) = delete;

  void update(const std::function<void()>& render);

  void poll_events();
  void swap_buffers();

  IWindowManager::ProcLoader get_proc_address() const;

  int get_height() const;
  int get_width() const;

  [[nodiscard]] bool should_close() const;

 private:
  GlfwWindow(ILogger& logger, const std::function<void()>& onShutdownRequested);

  const std::function<void()> onShutdownRequested_;

  ILogger& _logger;

  struct GlfwWindowDeleter {
    void operator()(GLFWwindow* window) const {
      if (window) {
        glfwDestroyWindow(window);
      }
    }
  };

  std::unique_ptr<GLFWwindow, GlfwWindowDeleter> window_;
};

}  // namespace screen_controller
