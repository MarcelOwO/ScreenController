//
// Created by marce on 4/2/2025.
//

#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include <logging/logger.h>

#include <functional>
#include <memory>

namespace screen_controller {

class IWindowManager {
 public:
  virtual ~IWindowManager() = default;

  virtual void update(const std::function<void()>& render) = 0;

  virtual void poll_events() = 0;
  virtual void swap_buffers() = 0;

  virtual int get_height() const = 0;
  virtual int get_width() const = 0;
  virtual bool should_close() const = 0;

  using ProcLoader = void* (*)(const char*);
  virtual ProcLoader get_proc_address() const = 0;
};

class WindowFactory {
 public:
  static std::unique_ptr<IWindowManager> Create(
      ILogger& logger, const std::function<void()> onShutdownRequested);
};

}  // namespace screen_controller

#endif  // WINDOW_MANAGER_H
