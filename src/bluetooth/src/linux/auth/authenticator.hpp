#pragma once

#include <openssl/crypto.h>
#include <openssl/evp.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <expected>
#include <span>
#include <string>
#include <string_view>

namespace screen_controller::auth {

inline constexpr std::size_t kKeySize{32};
inline constexpr std::size_t kNonceSize{32};
inline constexpr std::string_view kDomain{"screen-controller/auth/v2"};

using Key = std::array<uint8_t, kKeySize>;
using Nonce = std::array<uint8_t, kNonceSize>;
[[nodiscard]] inline int HexDigit(const char value) noexcept {
  if (value >= '0' && value <= '9') {
    return value - '0';
  }
  if (value >= 'a' && value <= 'f') {
    return value - 'a' + 10;
  }
  if (value >= 'A' && value <= 'F') {
    return value - 'A' + 10;
  }
  return -1;
}

[[nodiscard]] inline std::expected<Key, std::string> ParseKey(const std::string_view hex) {
  if (hex.size() != kKeySize * 2) {
    return std::unexpected("SCREEN_CONTROLLER_PSK_HEX must contain exactly 64 hex characters");
  }

  Key key{};
  for (std::size_t index = 0; index < key.size(); ++index) {
    const int high = HexDigit(hex[index * 2]);
    const int low = HexDigit(hex[(index * 2) + 1]);
    if (high < 0 || low < 0) {
      return std::unexpected("SCREEN_CONTROLLER_PSK_HEX contains a non-hex character");
    }
    key[index] = static_cast<uint8_t>((high << 4) | low);
  }
  return key;
}

[[nodiscard]] inline std::expected<Key, std::string> LoadKeyFromEnvironment() {
  const char* value = std::getenv("SCREEN_CONTROLLER_PSK_HEX");
  if (value == nullptr) {
    return std::unexpected(
        "SCREEN_CONTROLLER_PSK_HEX is not configured; refusing insecure Bluetooth startup");
  }
  return ParseKey(value);
}

[[nodiscard]] inline std::expected<std::array<uint8_t, 32>, std::string> ComputeResponse(
    const Key& key, const Nonce& nonce) {
  std::array<uint8_t, kDomain.size() + kNonceSize> message{};
  std::copy(kDomain.begin(), kDomain.end(), message.begin());
  std::copy(nonce.begin(), nonce.end(),
            message.begin() + static_cast<std::ptrdiff_t>(kDomain.size()));

  std::array<uint8_t, 32> response{};
  std::size_t response_size = 0;
  if (EVP_Q_mac(nullptr, "HMAC", nullptr, "SHA256", nullptr, key.data(), key.size(), message.data(),
                message.size(), response.data(), response.size(), &response_size) == nullptr) {
    return std::unexpected("OpenSSL failed to compute the authentication response");
  }
  if (response_size != response.size()) {
    return std::unexpected("OpenSSL returned an unexpected HMAC-SHA256 length");
  }
  return response;
}

[[nodiscard]] inline bool ValidateResponse(const Key& key, const Nonce& nonce,
                                           const std::span<const uint8_t> response) {
  const auto expected = ComputeResponse(key, nonce);
  return expected.has_value() && response.size() == expected->size() &&
         CRYPTO_memcmp(response.data(), expected->data(), expected->size()) == 0;
}

}  // namespace screen_controller::auth
