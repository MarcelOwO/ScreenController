//
// Created by marcel on 11/16/25.
//

#pragma once

#include <cstdint>

namespace screen_controller::bluetooth {

enum class ConnectionState : uint8_t {
  kStarting = 0,
  kAuthenticating = 1,
  kConnected = 2,
  kSending = 3,
  kDisconnected = 4,
};

}  // namespace screen_controller::bluetooth
