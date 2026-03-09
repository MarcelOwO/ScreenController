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

  static std::expected<std::unique_ptr<DefaultLogger>, std::error_code>
  create();

  void Log(LogLevel level, std::string_view log);

  DefaultLogger(const DefaultLogger&) = delete;
  DefaultLogger& operator=(const DefaultLogger&) = delete;
  DefaultLogger(DefaultLogger&&) = delete;
  DefaultLogger& operator=(DefaultLogger&&) = delete;

 private:
  DefaultLogger() = default;

  std::expected<void, std::error_code> setup_file();
  std::expected<std::string, std::error_code> get_file_name();

  std::string format_log(LogLevel level, std::string_view log);

  void run_thread(std::stop_token token);
  static std::string enum_to_string(const LogLevel level);

  static void log_internal(std::string_view msg);

  std::mutex log_queue_mutex_;
  std::condition_variable log_queue_cv_;

  std::queue<std::string> log_queue_;

  std::ofstream log_file_;

  std::jthread background_thread_;
};

}  // namespace screen_controller

#endif  // DEFAULT_LOGGER_H_
