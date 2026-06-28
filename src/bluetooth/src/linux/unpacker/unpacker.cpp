//
// Created by marce on 5/31/2025.
//

#include "unpacker.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <ranges>
#include <string>

#include "zstd.h"

namespace screen_controller::bluetooth {
Unpacker::Unpacker(ILogger& logger) : logger_(logger) {}
Unpacker::~Unpacker() = default;

void Unpacker::Decompress(const std::span<const std::byte> kSpan, BluetoothPacket& packet) const {
  const auto kType = static_cast<uint8_t>(kSpan[0]);
  packet.type = kType;

  const auto kNameLen = static_cast<size_t>(kSpan[1]);

  const auto kNameBytes = kSpan.subspan(2, kNameLen);
  std::string name(kNameBytes.size(), '\0');
  std::ranges::transform(kNameBytes, name.begin(),
                         [](std::byte byte) { return static_cast<char>(byte); });

  packet.name = name;
  logger_.LogInfo(name);
  if (kType == 1) {
    logger_.LogInfo("command");
    return;
  }
  logger_.LogInfo("File");

  static constexpr std::array kMagicNumber = {
      std::byte{0x28},
      std::byte{0xb5},
      std::byte{0x2f},
      std::byte{0xfd},
  };

  const auto kSearchSubrange = std::ranges::search(kSpan, kMagicNumber);

  if (kSearchSubrange.empty()) {
    logger_.LogInfo("Magic number not found in the span");
    return;
  }

  const auto kIndex = std::distance(kSpan.begin(), kSearchSubrange.begin());

  if (kIndex < 0 || kIndex >= static_cast<ptrdiff_t>(kSpan.size())) {
    logger_.LogFmt(LogLevel::ERROR, "Invalid index for magic number: {}", kIndex);
    return;
  }

  const auto kStartFile = static_cast<size_t>(kIndex);
  const std::span<const std::byte> src = kSpan.subspan(kStartFile);

  const size_t decompressed_size = ZSTD_getFrameContentSize(src.data(), src.size());
  if (decompressed_size == ZSTD_CONTENTSIZE_ERROR) {
    logger_.LogError("Not a valid compressed frame");
    return;
  }

  if (decompressed_size == ZSTD_CONTENTSIZE_UNKNOWN) {
    logger_.LogError("Compressed frame is unkown");
    return;
  }

  packet.data = std::vector<std::byte>(decompressed_size);

  const size_t result =
      ZSTD_decompress(packet.data.data(), decompressed_size, src.data(), src.size());

  if (ZSTD_isError(result)) {
    logger_.LogError("Decompression failed" + std::string(ZSTD_getErrorName(result)));
    return;
  }
  logger_.LogInfo("Decompressed successfully");
}
}  // namespace screen_controller::bluetooth
