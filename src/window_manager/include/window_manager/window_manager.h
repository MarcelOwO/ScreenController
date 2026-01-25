//
// Created by marce on 4/2/2025.
//

#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include <logging/logger.h>
#include <models/error_enum.h>

#include <expected>
#include <functional>
#include <memory>

namespace screen_controller {

class IWindowManager {
 public:
  virtual ~IWindowManager() = default;

  virtual void update(const std::function<void()>& render) const = 0;
  virtual void poll_events() const = 0;
  virtual void swap_buffers() const = 0;

  virtual int get_height() const = 0;
  virtual int get_width() const = 0;

  virtual bool should_close() const = 0;

 private:
  std::shared_ptr<Logger> logger_;
};

class WindowFactory {
 public:
  virtual std::unique_ptr<IWindowManager> Create(
      const std::shared_ptr<Logger>& logger, void* window_pointer) = 0;
};

}  // namespace screen_controller

#endif  // WINDOW_MANAGER_H
