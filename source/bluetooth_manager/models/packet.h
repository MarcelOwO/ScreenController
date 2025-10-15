//
// Created by marce on 05/08/2025.
//

#ifndef BLUETOOTHPACKET_H
#define BLUETOOTHPACKET_H

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
namespace screen_controller::bluetooth {
struct packet {
  uint8_t type{};
  std::string name;
  std::span<const std::byte> payload;
  uint32_t crc32{};
  bool has_payload{false};
};
}
#endif //BLUETOOTHPACKET_H
