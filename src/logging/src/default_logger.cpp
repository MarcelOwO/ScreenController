#include "default_logger.hpp"

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace screen_controller {

std::expected<std::unique_ptr<DefaultLogger>, std::error_code> DefaultLogger::Create() {
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  console_sink->set_level(spdlog::level::info);
  console_sink->set_pattern("[%H:%M:%S] [%^%l%$] %v");

  auto file_sink =
      std::make_shared<spdlog::sinks::basic_file_sink_mt>("screen_controller.log", true);
  file_sink->set_level(spdlog::level::trace);
  file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [thread %t] %v");

  spdlog::logger logger("screen_controller", {console_sink, file_sink});
  logger.set_level(spdlog::level::trace);

  logger.info(
      "\n==========================================\n"
      "=       Starting ScreenController        =\n"
      "=                  OwO                   =\n"
      "==========================================");

  return std::unique_ptr<DefaultLogger>(new DefaultLogger(std::move(logger)));
}

DefaultLogger::DefaultLogger(spdlog::logger logger) : logger_(std::move(logger)) {}

DefaultLogger::~DefaultLogger() = default;

void DefaultLogger::Log(const LogLevel level, const std::string_view message) {
  switch (level) {
    case LogLevel::TRACE:
      logger_.trace(message);
      break;
    case LogLevel::DEBUG:
      logger_.debug(message);
      break;
    case LogLevel::INFO:
      logger_.info(message);
      break;
    case LogLevel::WARN:
      logger_.warn(message);
      break;
    case LogLevel::ERROR:
      logger_.error(message);
      break;
  }
}

}  // namespace screen_controller
