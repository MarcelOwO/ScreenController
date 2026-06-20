#include "default_logger.hpp"

#include <expected>

#include "enums/log_level.hpp"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace screen_controller {

std::expected<std::unique_ptr<DefaultLogger>, std::error_code> DefaultLogger::Create() {
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

  console_sink->set_level(spdlog::level::warn);
  console_sink->set_pattern("[%H:%M:%S %z] [%n] [%^-%L-%$] [thread %t] %v");
  auto file_sink =
      std::make_shared<spdlog::sinks::basic_file_sink_mt>("screen_controller.log", true);
  file_sink->set_pattern("[%H:%M:%S %z] [%^-%L-%$] [thread %t] %v");
  file_sink->set_level(spdlog::level::trace);

  spdlog::logger logger("screen_controller", {console_sink, file_sink});

  logger.set_level(spdlog::level::debug);

  logger.info(
      "\n==========================================\n"
      "=                                        =\n"
      "=       Starting ScreenController        =\n"
      "=                                        =\n"
      "=                  OwO                   =\n"
      "=                                        =\n"
      "==========================================\n");

  auto logger_instance = std::unique_ptr<DefaultLogger>(new DefaultLogger(logger));

  return logger_instance;
}

DefaultLogger::~DefaultLogger() = default;

DefaultLogger::DefaultLogger(spdlog::logger logger) : logger_(std::move(logger)) {}

void DefaultLogger::Log(const LogLevel kLevel, const std::string_view kLog) {
  switch (kLevel) {
    case LogLevel::INFO: {
      logger_.info(kLog);
      break;
    }
    case LogLevel::ERROR: {
      logger_.error(kLog);
      break;
    }
  }
}

}  // namespace screen_controller
