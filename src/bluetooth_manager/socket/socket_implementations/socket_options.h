//
// Created by marce on 06/08/2025.
//

#ifndef SOCKETOPTIONS_H
#define SOCKETOPTIONS_H
#include <expected>
#include <memory>

#include "logging/logger.h"

namespace screen_controller {
class SocketOptions {
 public:
  explicit SocketOptions(const std::shared_ptr<Logger>& logger);
  ~SocketOptions() = default;

  SocketOptions(const SocketOptions&) = delete;
  SocketOptions& operator=(const SocketOptions&) = delete;
  SocketOptions(SocketOptions&&) = delete;
  SocketOptions& operator=(SocketOptions&&) = delete;

  [[nodiscard]] std::expected<void, std::error_code> SetSocketOption(int fd, int level,
                                                       int option, int value) const;

  [[nodiscard]] std::expected<void, std::error_code> SetReuseAddress(int fd, int value) const;
  [[nodiscard]] std::expected<void, std::error_code> SetReceiveBufferSize(int fd, int value)const ;
  [[nodiscard]] std::expected<void, std::error_code> SetSendBufferSize(int fd, int value)const ;
  [[nodiscard]] std::expected<void, std::error_code> SetFlushable(int fd, int value)const ;
  [[nodiscard]] std::expected<void, std::error_code> SetDefer(int fd, int value)const ;
  [[nodiscard]] std::expected<void, std::error_code> SetNonBlocking(int socket)const;

 private:
  std::shared_ptr<Logger> logger_;
};
}  // namespace screen_controller

#endif  // SOCKETOPTIONS_H
