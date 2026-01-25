//
// Created by marce on 06/08/2025.
//

#include "socket_helper.h"

namespace screen_controller {
void SocketHelper::BuildPacketBytes(const uint8_t type, const uint16_t kMagic,
                                    const std::string_view name,
                                    const std::span<const std::byte> payload,
                                    std::vector<uint8_t>& out_vec) {
  out_vec.clear();

  out_vec.reserve(1 + 2 + 4 + name.size() +
                  (payload.empty() ? 0 : 4 + 4 + payload.size()));

  auto push_le16 = [&](const uint16_t v) {
    out_vec.push_back(static_cast<uint8_t>(v & 0xFF));
    out_vec.push_back(static_cast<uint8_t>(v >> 8 & 0xFF));
  };
  auto push_le32 = [&](const uint32_t v) {
    out_vec.push_back(static_cast<uint8_t>(v & 0xFF));
    out_vec.push_back(static_cast<uint8_t>(v >> 8 & 0xFF));
    out_vec.push_back(static_cast<uint8_t>(v >> 16 & 0xFF));
    out_vec.push_back(static_cast<uint8_t>(v >> 24 & 0xFF));
  };

  push_le16(kMagic);
  out_vec.push_back(type);
  push_le32(static_cast<uint32_t>(name.size()));
  (void)out_vec.insert(out_vec.end(), name.begin(), name.end());

  if (!payload.empty()) {
    const auto p = reinterpret_cast<const uint8_t*>(payload.data());
    const uint32_t len = static_cast<uint32_t>(payload.size());
    const uint32_t crc = Crc32(p, len);
    push_le32(len);
    push_le32(crc);
    (void)out_vec.insert(out_vec.end(), p, p + len);
  }
}

uint32_t SocketHelper::Crc32(const uint8_t* data, const size_t size) {
  static std::array<uint32_t, 256> table;
  static bool inited = false;
  if (!inited) {
    for (uint32_t i = 0; i < 256; ++i) {
      uint32_t c = i;
      for (int k = 0; k < 8; ++k) {
        c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
      }
      table[i] = c;
    }
    inited = true;
  }
  uint32_t crc = 0xFFFFFFFFu;
  for (size_t i = 0; i < size; ++i) {
    crc = table[(crc ^ data[i]) & 0xFFu] ^ (crc >> 8);
  }
  return crc ^ 0xFFFFFFFFu;
}

uint16_t SocketHelper::le16(const uint8_t* p) {
  return static_cast<uint16_t>(p[0]) | (static_cast<uint16_t>(p[1]) << 8);
}
uint32_t SocketHelper::le32(const uint8_t* p) {
  return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
         (static_cast<uint32_t>(p[2]) << 16) |
         (static_cast<uint32_t>(p[3]) << 24);
}

}  // namespace screen_controller
