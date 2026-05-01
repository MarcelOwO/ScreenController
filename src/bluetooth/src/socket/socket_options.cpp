//
// Created by marce on 06/08/2025.
//

#include "socket_options.h"

#include <bluetooth/bluetooth.h>
#include <fcntl.h>
#include <sys/socket.h>

namespace screen_controller::socket {

expected SetSocketOption(int fd, int level, int option, int value) {
  if (int val = setsockopt(fd, level, option, &value, sizeof(int)); val < 0) {
    return std::unexpected(std::make_error_code(std::errc::operation_not_permitted));
  }
  return {};
}

expected SetReuseAddress(int fd, int value) {
  return SetSocketOption(fd, SOL_SOCKET, SO_REUSEADDR, value);
}

expected SetReceiveBufferSize(int fd, int value) {
  return SetSocketOption(fd, SOL_SOCKET, SO_RCVBUF, value);
}

expected SetSendBufferSize(int fd, int value) {
  return SetSocketOption(fd, SOL_SOCKET, SO_SNDBUF, value);
}

expected SetFlushable(int fd, int value) {
  return SetSocketOption(fd, SOL_BLUETOOTH, BT_FLUSHABLE, value);
}

expected SetDefer(int fd, int value) {
  return SetSocketOption(fd, SOL_BLUETOOTH, BT_DEFER_SETUP, value);
}

expected SetNonBlocking(int socket) {
  if (fcntl(socket, F_SETFL, O_NONBLOCK) < 0) {
    return std::unexpected(std::make_error_code(std::errc::operation_not_permitted));
  }

  return {};
}

}  // namespace screen_controller::socket
