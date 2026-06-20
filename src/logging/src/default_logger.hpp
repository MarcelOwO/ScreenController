#pragma once

#include <expected>

#include <spdlog/spdlog.h>
#include "logging/logger.hpp"

namespace screen_controller {

class DefaultLogger final : public ILogger {
public:
  ~DefaultLogger();

  static std::expected<std::unique_ptr<DefaultLogger>, std::error_code> Create();

  void Log(LogLevel level, std::string_view message) override;

  DefaultLogger(const DefaultLogger&) = delete;
  DefaultLogger& operator=(const DefaultLogger&) = delete;
  DefaultLogger(DefaultLogger&&) = delete;
  DefaultLogger& operator=(DefaultLogger&&) = delete;

private:
  explicit DefaultLogger(spdlog::logger logger);

  spdlog::logger logger_;
};

}  // namespace screen_controller
