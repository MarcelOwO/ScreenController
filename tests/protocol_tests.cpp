#include "socket_helper.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

using screen_controller::socket::BuildPacketBytes;
using screen_controller::socket::Crc;
using screen_controller::socket::FromSpan;

int main() {
  constexpr std::array<uint8_t, 2> kWord{0xBE, 0xEF};
  constexpr std::array<uint8_t, 4> kDword{0x01, 0x23, 0x45, 0x67};
  static_assert(FromSpan<uint16_t>(kWord) == 0xBEEF);
  static_assert(FromSpan<uint32_t>(kDword) == 0x01234567);

  constexpr std::array<uint8_t, 9> kCrcInput{'1', '2', '3', '4', '5', '6', '7', '8', '9'};
  static_assert(Crc(kCrcInput) == 0xCBF43926U);

  std::vector<std::byte> bytes;
  BuildPacketBytes(0x01, 0xBEEF, "Rotate", {}, bytes);
  const std::vector<std::byte> expected_command = {
      std::byte{0xBE}, std::byte{0xEF}, std::byte{0x01}, std::byte{0x00}, std::byte{0x00},
      std::byte{0x00}, std::byte{0x06}, std::byte{'R'},  std::byte{'o'},  std::byte{'t'},
      std::byte{'a'},  std::byte{'t'},  std::byte{'e'},
  };
  assert(bytes == expected_command);

  BuildPacketBytes(0xC2, 0xBEEF, "ok", {}, bytes);
  assert(bytes.size() == 17U);
  assert(bytes[9] == std::byte{0x00});
  assert(bytes[16] == std::byte{0x00});

  constexpr std::array payload{std::byte{0x10}, std::byte{0x20}};
  BuildPacketBytes(0x81, 0xBEEF, "x", payload, bytes);
  assert(bytes.size() == 18U);
  assert(bytes[8] == std::byte{0x00});
  assert(bytes[11] == std::byte{0x02});
  assert(bytes[16] == std::byte{0x10});
  assert(bytes[17] == std::byte{0x20});
}
