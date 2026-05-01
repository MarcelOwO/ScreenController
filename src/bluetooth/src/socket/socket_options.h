//
// Created by marce on 06/08/2025.
//

#ifndef SOCKETOPTIONS_H
#define SOCKETOPTIONS_H
#include <expected>
#include <system_error>

namespace screen_controller::socket {

using expected = std::expected<void, std::error_code>;

[[nodiscard]] expected SetSocketOption(int fd, int level, int option, int value);
[[nodiscard]] expected SetReuseAddress(int fd, int value);
[[nodiscard]] expected SetReceiveBufferSize(int fd, int value);
[[nodiscard]] expected SetSendBufferSize(int fd, int value);
[[nodiscard]] expected SetFlushable(int fd, int value);
[[nodiscard]] expected SetDefer(int fd, int value);
[[nodiscard]] expected SetNonBlocking(int socket);

}  // namespace screen_controller::socket

#endif  // SOCKETOPTIONS_H
