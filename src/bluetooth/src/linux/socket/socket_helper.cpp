//
// Created by marce on 06/08/2025.
//

#include "socket_helper.hpp"
#include <ranges>

namespace screen_controller::socket {

void BuildPacketBytes(uint8_t type, uint16_t magic, std::string_view name,
                      std::span<const std::byte> payload, std::vector<std::byte>& out) {
  out.clear();

  size_t total_size = sizeof(magic) + sizeof(type) + sizeof(uint32_t) + name.size();

  if (!payload.empty()) {
    total_size += sizeof(uint32_t) + sizeof(uint32_t) + payload.size();
  }

  out.reserve(total_size);

  Push16(out, magic);
  out.push_back(static_cast<std::byte>(type));

  Push32(out, static_cast<uint32_t>(name.size()));
  auto name_bytes = std::as_bytes(std::span{name});
  out.insert(out.end(), name_bytes.begin(), name_bytes.end());

  if (!payload.empty()) {
    const auto kLen = static_cast<uint32_t>(payload.size());

    auto uint8_view =
        payload | std::views::transform([](std::byte byte) { return static_cast<uint8_t>(byte); });

    const uint32_t kCrcVal = Crc(uint8_view);

    Push32(out, kLen);
    Push32(out, kCrcVal);

    out.insert(out.end(), payload.begin(), payload.end());
  }
}

uint16_t Le16(const std::array<std::byte, 2>& data) {
  return std::bit_cast<uint16_t>(data);
}

uint32_t Le32(const std::array<std::byte, 4>& data) {
  return std::bit_cast<uint32_t>(data);
}

void Push16(std::vector<std::byte>& vec, uint16_t value) {
  vec.insert(vec.end(), {static_cast<std::byte>(value >> 8), static_cast<std::byte>(value)});
}

void Push32(std::vector<std::byte>& vec, uint32_t value) {
  vec.insert(vec.end(), {static_cast<std::byte>(value >> 24), static_cast<std::byte>(value >> 16),
                         static_cast<std::byte>(value >> 8), static_cast<std::byte>(value)

                        });
}

}  // namespace screen_controller::socket
