//
// Created by marce on 06/08/2025.
//

#include "socket_options.h"

#include <bluetooth/bluetooth.h>
#include <fcntl.h>
#include <sys/socket.h>

namespace screen_controller {

SocketOptions::SocketOptions(const std::shared_ptr<Logger>& logger)
    : logger_(logger) {}

std::expected<void, std::error_code> SocketOptions::SetSocketOption(
    const int fd, const int level, const int option, const int value) const {
  if (const int val = setsockopt(fd, level, option, &value, sizeof(int));
      val < 0) {
    logger_->LogError("Failed to set socket option");
    return std::unexpected(
        std::make_error_code(std::errc::operation_not_permitted));
  }
  return {};
}

std::expected<void, std::error_code> SocketOptions::SetReuseAddress(
    const int fd, const int value) const {
  if (const auto res = SetSocketOption(fd, SOL_SOCKET, SO_REUSEADDR, value);
      !res) {
    logger_->LogError("Failed to set socket reuse address");
    return res;
  }
  return {};
}

std::expected<void, std::error_code> SocketOptions::SetReceiveBufferSize(
    const int fd, const int value) const {
  if (const auto res = SetSocketOption(fd, SOL_SOCKET, SO_RCVBUF, value);
      !res) {
    logger_->LogError("Failed to set socket receive buffer size");
    return res;
  }
  return {};
}

std::expected<void, std::error_code> SocketOptions::SetSendBufferSize(
    const int fd, const int value) const {
  if (const auto res = SetSocketOption(fd, SOL_SOCKET, SO_SNDBUF, value);
      !res) {
    logger_->LogError("Failed to set socket send buffer size");
    return res;
  }
  return {};
}

std::expected<void, std::error_code> SocketOptions::SetFlushable(
    const int fd, const int value) const {
  if (const auto res = SetSocketOption(fd, SOL_BLUETOOTH, BT_FLUSHABLE, value);
      !res) {
    logger_->LogError("Failed to set socket flushable");
    return res;
  }
  return {};
}

std::expected<void, std::error_code> SocketOptions::SetDefer(
    const int fd, const int value) const {
  if (const auto res =
          SetSocketOption(fd, SOL_BLUETOOTH, BT_DEFER_SETUP, value);
      !res) {
    logger_->LogError("Failed to set socket defer");
    return res;
  }
  return {};
}

std::expected<void, std::error_code> SocketOptions::SetNonBlocking(
    const int socket) const {
  if (fcntl(socket, F_SETFL, O_NONBLOCK) < 0) {
    logger_->LogError("Failed to set non-blocking mode");
    return std::unexpected(std::make_error_code(std::errc::operation_not_permitted));
  }

  return {};
}

}  // namespace screen_controller
