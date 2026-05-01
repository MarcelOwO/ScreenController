//
// Created by marcel on 11/16/25.
//

#ifndef SCREENCONTROLLER_CONNECTIONSTATE_H
#define SCREENCONTROLLER_CONNECTIONSTATE_H

#include <cstdint>

enum class ConnectionState : uint8_t {
  kStarting = 0,
  kNotConnected = 1,
  kConnected = 2,
  kSending = 3,
  kDisconnected = 4,
};

#endif  // SCREENCONTROLLER_CONNECTIONSTATE_H
