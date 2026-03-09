#include "default_logger.h"

#include <expected>
#include <filesystem>
#include <iostream>
#include <print>

namespace screen_controller {

std::expected<std::unique_ptr<DefaultLogger>, std::error_code>
DefaultLogger::create() {
  auto logger = std::unique_ptr<DefaultLogger>(new DefaultLogger());

  if (auto err = logger->setup_file(); !err) {
    log_internal("Failed to setup setup file in logger creation");
    return std::unexpected(err.error());
  }

  logger->background_thread_ =
      std::jthread([logger = logger.get()](std::stop_token st) {
        logger->run_thread(st);
        log_internal("logger thread shutting down");
      });

  logger->Log(LogLevel::INFO,
              "\n==========================================\n"
              "=                                        =\n"
              "=       Starting ScreenController        =\n"
              "=                                        =\n"
              "=                  OwO                   =\n"
              "=                                        =\n"
              "==========================================\n");
  return logger;
}

DefaultLogger::~DefaultLogger() = default;

std::expected<void, std::error_code> DefaultLogger::setup_file() {
  const std::filesystem::path log_dir = "logs";
  std::error_code ec;

  if (!std::filesystem::exists(log_dir, ec)) {
    if (!std::filesystem::create_directory(log_dir, ec)) {
      log_internal("failed to create directory");
      return std::unexpected(ec);
    }
  }

  auto now = std::chrono::system_clock::now();
  std::time_t now_c = std::chrono::system_clock::to_time_t(now);

  std::tm tm_buf;
  localtime_r(&now_c, &tm_buf);

  std::string filename =
      std::format("logs/logs_{:04}-{:02}-{:02}.log", tm_buf.tm_year + 1900,
                  tm_buf.tm_mon + 1, tm_buf.tm_mday);

  log_file_.open(filename, std::ios::app);
  if (!log_file_.is_open()) {
    log_internal("Logger: File is not open");
    return std::unexpected(std::make_error_code(std::errc::io_error));
  }

  return {};
}

std::string DefaultLogger::format_log(LogLevel level, std::string_view log) {
  auto now = std::chrono::system_clock::now();
  std::time_t now_c = std::chrono::system_clock::to_time_t(now);

  std::tm tm_buf;
  localtime_r(&now_c, &tm_buf);

  char time_str[9];
  std::strftime(time_str, sizeof(time_str), "%H:%M:%S", &tm_buf);

  return std::format("{} | {} | {}", time_str, enum_to_string(level), log);
}

void DefaultLogger::run_thread(std::stop_token token) {
  while (true) {
    std::string msg;
    {
      std::unique_lock<std::mutex> lock(log_queue_mutex_);

      log_queue_cv_.wait(lock, [this, &token] {
        return !log_queue_.empty() || token.stop_requested();
      });

      if (token.stop_requested() && log_queue_.empty()) {
        log_internal("Logger stopped");
        break;
      }

      msg = std::move(log_queue_.front());

      log_queue_.pop();
    }

    if (log_file_.is_open()) {
      log_file_ << msg << std::endl;
      std::println("{}", msg);
    } else {
      log_internal("Log file is not open");
    }
  }
}

std::string DefaultLogger::enum_to_string(const LogLevel level) {
  switch (level) {
    case LogLevel::INFO:
      return "INFO";
    case LogLevel::ERROR:
      return "Error";
    default:
      return "Unknown";
  };
}

void DefaultLogger::Log(const LogLevel level, const std::string_view log) {
  const std::string log_message = format_log(level, log);

  {
    std::lock_guard<std::mutex> lock(log_queue_mutex_);
    log_queue_.push(log_message);
  }

  log_queue_cv_.notify_one();
}

void DefaultLogger::log_internal(std::string_view msg) {
  std::println("INTERNAL_LOGGER | {}", msg);
}

}  // namespace screen_controller
