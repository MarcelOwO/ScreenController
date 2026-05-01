//
// Created by marce on 06/08/2025.
//

#include "socket_helper.h"
#include <algorithm>
#include <numeric>
#include <ranges>

#include "socket_variables.h"

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

    const uint32_t kCrcVal = crc(uint8_view);

    Push32(out, kLen);
    Push32(out, kCrcVal);

    out.insert(out.end(), payload.begin(), payload.end());
  }
}

[[nodiscard]] constexpr std::uint32_t crc(const std::ranges::input_range auto& rng) noexcept
  requires std::convertible_to<std::ranges::range_value_t<decltype(rng)>, std::uint8_t>
{
  return ~std::accumulate(std::ranges::begin(rng), std::ranges::end(rng),
                          ~std::uint32_t{0} & std::uint32_t{0xff'ff'ff'ffu},
                          [](std::uint32_t checksum, std::uint8_t value) {
                            return crc_table[(checksum ^ value) & 0xff] ^ (checksum >> 8);
                          });
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
