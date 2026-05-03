//
// Created by marce on 06/08/2025.
//

#ifndef SOCKETHELPER_H
#define SOCKETHELPER_H

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <span>
#include <string_view>
#include <vector>

namespace screen_controller::socket {

void BuildPacketBytes(uint8_t type, uint16_t magic, std::string_view name,
                      std::span<const std::byte> payload, std::vector<std::byte>& out_vec);

uint16_t Le16(const std::array<std::byte, 2>& data);
uint32_t Le32(const std::array<std::byte, 4>& data);

void Push16(std::vector<std::byte>& vec, uint16_t value);
void Push32(std::vector<std::byte>& vec, uint32_t value);

template <std::integral T>
T FromSpan(std::span<const uint8_t, sizeof(T)> data) {
  return std::bit_cast<T>(*reinterpret_cast<const std::array<uint8_t, sizeof(T)>*>(data.data()));
}

template <std::integral T>
T FromVector(const std::vector<uint8_t>& data, size_t offset) {
  auto part = std::span<const uint8_t>(data).subspan(offset).first<sizeof(T)>();
  return FromSpan<T>(part);
}

inline constexpr auto kCrcTable = []() {
  std::array<std::uint32_t, 256> retval{};
  std::ranges::generate(retval, [n = std::uint32_t{0}]() mutable {
    auto c = n++;
    for (std::uint8_t k = 0; k < 8; ++k) {
      if (c & 1)
        c = std::uint32_t{0xedb88320} ^ (c >> 1);
      else
        c >>= 1;
    }
    return c;
  });
  return retval;
}();

[[nodiscard]] constexpr std::uint32_t Crc(const std::ranges::input_range auto& rng) noexcept
  requires std::convertible_to<std::ranges::range_value_t<decltype(rng)>, std::uint8_t>
{
  return ~std::accumulate(std::ranges::begin(rng), std::ranges::end(rng),
                          std::uint32_t{0xff'ff'ff'ffU} & ~std::uint32_t{0},
                          [](std::uint32_t checksum, std::uint8_t value) {
                            return (checksum >> 8) ^ kCrcTable[(checksum ^ value) & 0xff];
                          });
}

}  // namespace screen_controller::socket

#endif  // SOCKETHELPER_H
