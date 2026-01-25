//
// Created by marce on 5/31/2025.
//

#ifndef BLUETOOTH_PACKET_H
#define BLUETOOTH_PACKET_H

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace screen_controller::common {
struct BluetoothPacket {
  uint8_t type;
  std::string name;
  size_t size;
  std::vector<std::byte> data;
};
};  // namespace screen_controller::common
#endif  // BLUETOOTH_PACKET_H
