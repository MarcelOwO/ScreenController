//
// Created by marcel on 11/16/25.
//

#pragma once

#include <cstdint>

enum class ConnectionState : uint8_t {
  kStarting = 0,
  kNotConnected = 1,
  kConnected = 2,
  kSending = 3,
  kDisconnected = 4,
};
