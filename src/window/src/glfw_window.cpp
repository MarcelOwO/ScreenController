#include "glfw_window.hpp"

#include <format>

#include "GLFW/glfw3.h"

namespace screen_controller {

std::expected<std::unique_ptr<GlfwWindow>, std::error_code> GlfwWindow::Create(
    ILogger& logger, std::function<void()> on_shutdown_requested) {
  logger.LogInfo("Creating WindowManager");

  if (!on_shutdown_requested) {
    return std::unexpected(std::make_error_code(std::errc::invalid_argument));
  }

  if (instance_ != nullptr) {
    logger.LogError("Tried to create a second window");
    return std::unexpected(std::make_error_code(std::errc::invalid_argument));
  }

  glfwSetErrorCallback([](int, const char* description) {
    auto* window = GlfwWindow::instance_;
    if (window == nullptr) {
      return;
    }
    const auto kFormatted = std::format("Error in glfw callback: {}", description);
    window->logger_.LogError(kFormatted);
  });

  if (glfwInit() != GLFW_TRUE) {
    logger.LogError("Failed to initialize GLFW");
    return std::unexpected(std::make_error_code(std::errc::connection_refused));
  }

#if defined(__linux__)
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
  glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#else
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

  auto* raw_window = glfwCreateWindow(1920, 1080, "My Title", glfwGetPrimaryMonitor(), nullptr);

  if (raw_window == nullptr) {
    const char* description = nullptr;
    const int code = glfwGetError(&description);
    logger.LogFmt(LogLevel::ERROR, "Failed to create GLFW window ({}): {}", code,
                  description == nullptr ? "unknown error" : description);
    return std::unexpected(std::make_error_code(std::errc::invalid_argument));
  }

  glfwMakeContextCurrent(raw_window);

  glfwSetKeyCallback(raw_window,
                     [](GLFWwindow* window, int key, int scancode, int action, int mods) {
                       if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
                         auto* const kRef = GlfwWindow::instance_;

                         if (kRef == nullptr) {
                           return;
                         }

                         kRef->on_shutdown_requested_();
                       }
                     });

  glfwSwapInterval(1);

  auto window =
      std::unique_ptr<GlfwWindow>(new GlfwWindow(logger, std::move(on_shutdown_requested)));

  window->window_ = std::unique_ptr<GLFWwindow, GlfwWindowDeleter>(raw_window);

  instance_ = window.get();

  return window;
}

GlfwWindow::GlfwWindow(ILogger& logger, std::function<void()> on_shutdown_requested)
    : logger_(logger), on_shutdown_requested_(std::move(on_shutdown_requested)) {}

bool GlfwWindow::ShouldClose() const {
  return glfwWindowShouldClose(window_.get()) != 0;
}

void GlfwWindow::PollEvents() {
  glfwPollEvents();
}

int GlfwWindow::GetHeight() const {
  const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
  return (mode != nullptr) ? mode->height : 1080;
}

int GlfwWindow::GetWidth() const {
  const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
  return (mode != nullptr) ? mode->width : 1920;
}

IWindowManager::ProcLoader GlfwWindow::GetProcAddress() const {
  return reinterpret_cast<IWindowManager::ProcLoader>(glfwGetProcAddress);
}

void GlfwWindow::Update(const std::function<void()>& render) {
  if (window_ == nullptr) {
    return;
  }

  render();

  glfwSwapBuffers(window_.get());
}

GlfwWindow::~GlfwWindow() {
  logger_.LogInfo("Cleaning up Window");
  instance_ = nullptr;

  glfwTerminate();
  logger_.LogInfo("Cleaned up window");
}

}  // namespace screen_controller
