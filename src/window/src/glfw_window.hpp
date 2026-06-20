
#include <logging/logger.hpp>
#include <system_error>
#include <window_manager/window_manager.hpp>

#include <expected>

#include "GLFW/glfw3.h"

namespace screen_controller {

class GlfwWindow : public IWindowManager {
public:
  inline static GlfwWindow* instance_ = nullptr;

  static std::expected<std::unique_ptr<GlfwWindow>, std::error_code> Create(
      ILogger& logger, std::function<void()> on_shutdown_requested);

  ~GlfwWindow() override;

  GlfwWindow(const GlfwWindow&) = delete;
  GlfwWindow& operator=(const GlfwWindow&) = delete;
  GlfwWindow(GlfwWindow&&) = delete;
  GlfwWindow& operator=(GlfwWindow&&) = delete;

  void update(const std::function<void()>& render) override;

  void poll_events() override;
  void swap_buffers() override;

  [[nodiscard]] IWindowManager::ProcLoader get_proc_address() const override;

  [[nodiscard]] int get_height() const override;
  [[nodiscard]] int get_width() const override;

  [[nodiscard]] bool should_close() const override;

private:
  GlfwWindow(ILogger& logger, const std::function<void()>& on_shutdown_requested);

  const std::function<void()> kOnShutdownRequested;

  ILogger& logger_;

  struct GlfwWindowDeleter {
    void operator()(GLFWwindow* window) const {
      if (window != nullptr) {
        glfwDestroyWindow(window);
      }
    }
  };

  std::unique_ptr<GLFWwindow, GlfwWindowDeleter> window_;
};

}  // namespace screen_controller
