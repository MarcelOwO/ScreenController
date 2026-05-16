#include "default_logger.h"

#include <expected>
#include <filesystem>
#include <iostream>
#include <print>
#include <utility>

namespace screen_controller {

std::expected<std::unique_ptr<DefaultLogger>, std::error_code> DefaultLogger::Create() {
  auto logger = std::unique_ptr<DefaultLogger>(new DefaultLogger());

  auto err = logger->SetupFile();

  if (!err) {
    LogInternal("Failed to setup setup file in logger creation");
    return std::unexpected(err.error());
  }

  logger->StartBackgroundThread();

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

void DefaultLogger::StartBackgroundThread() {
  background_thread_ = std::jthread([this](std::stop_token token) {
    RunThread(std::move(token));
    LogInternal("logger thread shutting down");
  });
}

std::expected<void, std::error_code> DefaultLogger::SetupFile() {
  const std::filesystem::path kLogDir = "logs";
  std::error_code err;

  if (!std::filesystem::exists(kLogDir, err)) {
    if (!std::filesystem::create_directory(kLogDir, err)) {
      LogInternal("failed to create directory");
      return std::unexpected(err);
    }
  }

  auto now = std::chrono::system_clock::now();
  std::time_t now_c = std::chrono::system_clock::to_time_t(now);

  std::tm tm_buf;
  localtime_r(&now_c, &tm_buf);

  std::string filename = std::format("logs/logs_{:04}-{:02}-{:02}.log", tm_buf.tm_year + 1900,
                                     tm_buf.tm_mon + 1, tm_buf.tm_mday);

  log_file_.open(filename, std::ios::app);
  if (!log_file_.is_open()) {
    LogInternal("Logger: File is not open");
    return std::unexpected(std::make_error_code(std::errc::io_error));
  }

  return {};
}

std::string DefaultLogger::FormatLog(LogLevel level, std::string_view log) {
  auto now = std::chrono::system_clock::now();
  std::time_t now_c = std::chrono::system_clock::to_time_t(now);

  std::tm tm_buf;
  localtime_r(&now_c, &tm_buf);

  char time_str[9];
  std::strftime(time_str, sizeof(time_str), "%H:%M:%S", &tm_buf);

  return std::format("{} | {} | {}", time_str, EnumToString(level), log);
}

void DefaultLogger::RunThread(std::stop_token token) {
  while (true) {
    std::string msg;
    {
      std::unique_lock<std::mutex> lock(log_queue_mutex_);

      log_queue_cv_.wait(lock,
                         [this, &token] { return !log_queue_.empty() || token.stop_requested(); });

      if (token.stop_requested() && log_queue_.empty()) {
        LogInternal("Logger stopped");
        break;
      }

      msg = std::move(log_queue_.front());

      log_queue_.pop();
    }

    if (log_file_.is_open()) {
      log_file_ << msg << std::endl;
      std::println("{}", msg);
    } else {
      LogInternal("Log file is not open");
    }
  }
}

std::string DefaultLogger::EnumToString(const LogLevel kLevel) {
  switch (kLevel) {
    case LogLevel::INFO:
      return "INFO";
    case LogLevel::ERROR:
      return "Error";
    default:
      return "Unknown";
  };
}

void DefaultLogger::Log(const LogLevel kLevel, const std::string_view kLog) {
  const std::string kLogMessage = FormatLog(kLevel, kLog);

  {
    std::lock_guard<std::mutex> lock(log_queue_mutex_);
    log_queue_.push(kLogMessage);
  }

  log_queue_cv_.notify_one();
}

void DefaultLogger::LogInternal(std::string_view msg) {
  std::println("INTERNAL_LOGGER | {}", msg);
}

}  // namespace screen_controller
