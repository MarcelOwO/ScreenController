//
// Created by marce on 06/08/2025.
//

#include "socket_options.h"

#include <bluetooth/bluetooth.h>
#include <fcntl.h>
#include <sys/socket.h>

#include "ng-log/logging.h"

namespace screen_controller::bluetooth {

bool SocketOptions::SetSocketOption(const int fd, const int level,
                                   const int option, const int value) {
  const int val = setsockopt(fd, level, option, &value, sizeof(int));
  return val >= 0;
}

void SocketOptions::SetReuseAddress(const int fd, const int value) {
  CHECK(SetSocketOption(fd, SOL_SOCKET, SO_REUSEADDR, value))
      << "Failed to set socket to reuse address";
}

void SocketOptions::SetReceiveBufferSize(const int fd, const int value) {
  CHECK(SetSocketOption(fd, SOL_SOCKET, SO_RCVBUF, value))
      << "Failed to set socket receive buffer size";
}

void SocketOptions::SetSendBufferSize(const int fd, const int value) {
  CHECK(SetSocketOption(fd, SOL_SOCKET, SO_SNDBUF, value))
      << "Failed to set socket send buffer size";
}
void SocketOptions::SetFlushable(const int fd, const int value) {
  CHECK(SetSocketOption(fd, SOL_BLUETOOTH, BT_FLUSHABLE, value))
      << "Failed to set socket flushable";
}
void SocketOptions::SetDefer(const int fd, const int value) {
  CHECK(SetSocketOption(fd, SOL_BLUETOOTH, BT_DEFER_SETUP, value))
      << "Failed to set socket defer";
}
void SocketOptions::SetNonBlocking(int socket) {
  CHECK(fcntl(socket, F_SETFL, O_NONBLOCK) >= 0)
      << "Failed to set non-blocking mode";
}


}