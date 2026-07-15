#include "authenticator.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <string>

using screen_controller::auth::ComputeResponse;
using screen_controller::auth::Key;
using screen_controller::auth::Nonce;
using screen_controller::auth::ParseKey;
using screen_controller::auth::ValidateResponse;

int main() {
  assert(!ParseKey("short").has_value());
  assert(!ParseKey(std::string(64, 'z')).has_value());

  Key key{};
  Nonce nonce{};
  for (std::size_t index = 0; index < key.size(); ++index) {
    key[index] = static_cast<uint8_t>(index);
    nonce[index] = static_cast<uint8_t>(index + 32U);
  }

  const auto response = ComputeResponse(key, nonce);
  assert(response.has_value());
  constexpr std::array<uint8_t, 32> kExpected = {
      0x59, 0xE7, 0xC4, 0xB8, 0x69, 0x6D, 0x4E, 0x0A, 0x02, 0xD1, 0xB4,
      0xEE, 0x91, 0x16, 0x67, 0x04, 0x54, 0xBA, 0x8B, 0xAD, 0xE5, 0xB7,
      0xC6, 0xC1, 0x94, 0x83, 0xDA, 0x89, 0xB3, 0x86, 0x2E, 0x57,
  };
  assert(*response == kExpected);
  assert(ValidateResponse(key, nonce, *response));

  auto tampered = *response;
  tampered.front() ^= 0x01U;
  assert(!ValidateResponse(key, nonce, tampered));
}
