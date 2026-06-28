#pragma once

#include <cstddef>
#include <cstdint>

namespace screen_controller::socket {

inline constexpr uint16_t kMagic{0xBEEF};
inline constexpr std::size_t kHeaderMin{2 + 1 + 4};
inline constexpr uint32_t kMaxNameLen{256U};
inline constexpr uint32_t kMaxPayload{64U * 1024U * 1024U};  // 64 MB

// ── Packet type constants ─────────────────────────────────────────────────────
// Phone → Device (type < 0x80 = command/no-payload; type >= 0x80 = has payload)
inline constexpr uint8_t kTypeCommand{0x01};       // name = command string
inline constexpr uint8_t kTypeAuthResponse{0x80};  // payload = 32-byte PSK response
inline constexpr uint8_t kTypeFileTransfer{0x81};  // payload = zstd-compressed file

// Device → Phone (type >= 0xC0, always have payload)
inline constexpr uint8_t kTypeChallenge{0xC0};    // payload = 16-byte nonce
inline constexpr uint8_t kTypeFileList{0xC1};     // payload = newline-separated filenames
inline constexpr uint8_t kTypeAck{0xC2};          // payload = empty, name = ack-tag
inline constexpr uint8_t kTypeDeviceError{0xCF};  // payload = error message string

}  // namespace screen_controller::socket
