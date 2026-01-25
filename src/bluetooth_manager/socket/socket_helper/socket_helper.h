//
// Created by marce on 06/08/2025.
//

#ifndef SOCKETHELPER_H
#define SOCKETHELPER_H

#include <cstdint>
#include <span>
#include <string_view>
#include <vector>
namespace screen_controller {
class SocketHelper {
 public:
  static void BuildPacketBytes(uint8_t type, uint16_t kMagic,
                               std::string_view name,
                               std::span<const std::byte> payload,
                               std::vector<uint8_t>& out_vec);

  static uint32_t Crc32(const uint8_t* data, size_t size);
  static uint16_t le16(const uint8_t* p);
  static uint32_t le32(const uint8_t* p);
};

}  // namespace screen_controller

#endif  // SOCKETHELPER_H
