#pragma once

#include <cstdint>

namespace screen_controller::socket {

inline constexpr uint8_t kMask8bit{0xFF};

inline constexpr uint16_t kMagic{0xBEEF};
inline constexpr std::size_t kHeaderMin{2 + 1 + 4};
inline constexpr uint32_t kMaxNameLen{1U + 20};
inline constexpr uint32_t kMaxPayload{16U + 20};

}  // namespace screen_controller::socket
