//
// Created by marce on 05/08/2025.
//

#ifndef BLUETOOTHPACKET_H
#define BLUETOOTHPACKET_H

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>

namespace screen_controller {

struct Packet {
  uint8_t type_{};
  std::string name_;
  std::span<const std::byte> payload_;
  uint32_t crc32_{};
  bool has_payload_{false};
};

}  // namespace screen_controller
#endif  // BLUETOOTHPACKET_H
