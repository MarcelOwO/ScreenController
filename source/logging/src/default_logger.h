#ifndef DEFAULT_LOGGER_H
#define DEFAULT_LOGGER_H

#include <condition_variable>
#include <expected>
#include <fstream>
#include <queue>
#include <thread>

#include "../include/logging/logger.h"

namespace screen_controller {

class DefaultLogger final : public ILogger {
public:
  ~DefaultLogger();

  static std::expected<std::unique_ptr<DefaultLogger>, std::error_code> Create();

  void Log(LogLevel level, std::string_view log) override;

  DefaultLogger(const DefaultLogger&) = delete;
  DefaultLogger& operator=(const DefaultLogger&) = delete;
  DefaultLogger(DefaultLogger&&) = delete;
  DefaultLogger& operator=(DefaultLogger&&) = delete;

private:
  DefaultLogger() = default;

  std::expected<void, std::error_code> SetupFile();
  std::expected<std::string, std::error_code> GetFileName();

  std::string FormatLog(LogLevel level, std::string_view log);

  void RunThread(std::stop_token token);
  static std::string EnumToString(LogLevel level);

  static void LogInternal(std::string_view msg);

  void StartBackgroundThread();

  std::mutex log_queue_mutex_;
  std::condition_variable log_queue_cv_;

  std::queue<std::string> log_queue_;

  std::ofstream log_file_;

  std::jthread background_thread_;
};

}  // namespace screen_controller

#endif  // DEFAULT_LOGGER_H_
