#pragma once

#include <array>
#include <cstdint>
#include <span>

namespace screen_controller::auth {

// Pre-shared key embedded in both this app and the phone app.
// Phone proves knowledge of the PSK by responding to a nonce challenge without
// transmitting the PSK itself: response[i] = PSK[i] XOR nonce[i % 16].
constexpr std::array<uint8_t, 32> kAppPsk = {
    0x4F, 0x77, 0x4F, 0x42, 0x61, 0x63, 0x6B, 0x70,  // OwOBackp
    0x61, 0x63, 0x6B, 0x43, 0x6F, 0x6E, 0x74, 0x72,  // ackContr
    0x6F, 0x6C, 0x6C, 0x65, 0x72, 0x32, 0x30, 0x32,  // oller202
    0x35, 0x4F, 0x57, 0x4F, 0x21, 0x4F, 0x57, 0x4F,  // 5OWO!OWO
};

// Compute the expected auth response given the nonce the device sent.
// response[i] = PSK[i] XOR nonce[i % 16]
[[nodiscard]] inline std::array<uint8_t, 32> ComputeResponse(
    const std::array<uint8_t, 16>& nonce) noexcept {
  std::array<uint8_t, 32> response{};
  for (size_t i = 0; i < 32; ++i) {
    response[i] = kAppPsk[i] ^ nonce[i % 16];
  }
  return response;
}

// Validate a response received from the phone against the nonce we sent.
[[nodiscard]] inline bool ValidateResponse(const std::array<uint8_t, 16>& nonce,
                                           std::span<const uint8_t> response) noexcept {
  if (response.size() != kAppPsk.size()) {
    return false;
  }
  for (size_t i = 0; i < kAppPsk.size(); ++i) {
    if ((response[i] ^ nonce[i % 16]) != kAppPsk[i]) {
      return false;
    }
  }
  return true;
}

}  // namespace screen_controller::auth
