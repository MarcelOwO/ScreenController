//
// Created by marce on 06/08/2025.
//

#pragma once

#include <expected>
#include <system_error>

namespace screen_controller::socket {

using expected = std::expected<void, std::error_code>;

[[nodiscard]] expected SetSocketOption(int file_descriptor, int level, int option, int value);
[[nodiscard]] expected SetReuseAddress(int file_descriptor, int value);
[[nodiscard]] expected SetReceiveBufferSize(int file_descriptor, int value);
[[nodiscard]] expected SetSendBufferSize(int file_descriptor, int value);
[[nodiscard]] expected SetFlushable(int file_descriptor, int value);
[[nodiscard]] expected SetBluetoothSecurity(int file_descriptor, int level);
[[nodiscard]] expected SetNonBlocking(int socket);

}  // namespace screen_controller::socket
