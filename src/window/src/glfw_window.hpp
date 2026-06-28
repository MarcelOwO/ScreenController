
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

  void Update(const std::function<void()>& render) override;

  void PollEvents() override;

  [[nodiscard]] IWindowManager::ProcLoader GetProcAddress() const override;

  [[nodiscard]] int GetHeight() const override;
  [[nodiscard]] int GetWidth() const override;

  [[nodiscard]] bool ShouldClose() const override;

private:
  GlfwWindow(ILogger& logger, std::function<void()> on_shutdown_requested);

  std::function<void()> on_shutdown_requested_;

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
