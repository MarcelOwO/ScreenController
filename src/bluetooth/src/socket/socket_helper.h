//
// Created by marce on 06/08/2025.
//

#ifndef SOCKETHELPER_H
#define SOCKETHELPER_H

#include <algorithm>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace screen_controller::socket {

void BuildPacketBytes(uint8_t type, uint16_t magic, std::string_view name,
                      std::span<const std::byte> payload, std::vector<uint8_t>& out_vec);

[[nodiscard]] constexpr std::uint32_t crc(const std::ranges::input_range auto& rng) noexcept
  requires std::convertible_to<std::ranges::range_value_t<decltype(rng)>, std::uint8_t>;

uint16_t Le16(const uint8_t* pointer);
uint32_t Le32(const uint8_t* pointer);

void Push16(std::vector<std::byte>& vec, uint16_t value);
void Push32(std::vector<std::byte>& vec, uint32_t value);

inline constexpr auto crc_table = []() {
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

}  // namespace screen_controller::socket

#endif  // SOCKETHELPER_H
