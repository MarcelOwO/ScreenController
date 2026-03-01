
#include <logging/logger.h>
#include <window_manager/window_manager.h>

#include "GLFW/glfw3.h"

namespace screen_controller {

class GlfwWindow : public IWindowManager {
 public:
  explicit GlfwWindow(ILogger& logger,
                      const std::function<void()>& onShutdownRequested);
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

  inline static GlfwWindow* instance = nullptr;

 private:
  const std::function<void()>& onShutdownRequested_;
  GLFWwindow* window_;
  ILogger& _logger;
};
}  // namespace screen_controller
