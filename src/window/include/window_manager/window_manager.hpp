//
// Created by marce on 4/2/2025.
//

#pragma once

#include <logging/logger.hpp>

#include <functional>
#include <memory>

namespace screen_controller {

class IWindowManager {
public:
  virtual ~IWindowManager() = default;

  virtual void Update(const std::function<void()>& render) = 0;

  virtual void PollEvents() = 0;

  [[nodiscard]] virtual int GetHeight() const = 0;
  [[nodiscard]] virtual int GetWidth() const = 0;
  [[nodiscard]] virtual bool ShouldClose() const = 0;

  using ProcLoader = void* (*) (const char*);
  [[nodiscard]] virtual ProcLoader GetProcAddress() const = 0;
};

class WindowFactory {
public:
  static std::unique_ptr<IWindowManager> Create(ILogger& logger,
                                                std::function<void()> on_shutdown_requested);
};

}  // namespace screen_controller
