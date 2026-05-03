//
// Created by marce on 5/31/2025.
//

#include "unpacker.h"

#include <bits/ranges_util.h>

#include "zstd.h"

namespace screen_controller::bluetooth {
Unpacker::Unpacker(ILogger& logger) : logger_(logger) {}
Unpacker::~Unpacker() = default;

void Unpacker::Decompress(const std::span<std::byte> kSpan, common::BluetoothPacket& packet) const {
  std::vector<char> debug_string{};

  const auto kType = static_cast<uint8_t>(kSpan[static_cast<size_t>(0)]);

  debug_string.push_back(static_cast<char>(kType));
  debug_string.push_back(' ');

  packet.type = kType;
  const int kLenString = static_cast<int>(kSpan[static_cast<size_t>(1)]);

  debug_string.push_back(static_cast<char>(kLenString));
  debug_string.push_back(' ');

  std::vector<char> name_vector(kLenString);

  for (int i = 0; i < kLenString; ++i) {
    name_vector[i] = static_cast<char>(kSpan[i + 2]);
  }

  const auto kName = std::string(name_vector.data(), name_vector.size());

  packet.name = kName;
  logger_.LogInfo(kName);
  if (kType == 1) {
    logger_.LogInfo("command");
    return;
  }
  logger_.LogInfo("File");

  std::vector<std::byte> magic_number = {
      std::byte{0x28},
      std::byte{0xb5},
      std::byte{0x2f},
      std::byte{0xfd},
  };

  const auto search_subrange = std::ranges::search(kSpan, magic_number);

  if (search_subrange.empty()) {
    logger_.LogInfo("Magic number not found in the span");
    return;
  }

  const auto index = std::distance(kSpan.begin(), search_subrange.begin());

  if (index < 0 || index >= static_cast<int>(kSpan.size())) {
    logger_.LogError("Invalid index for magic number:" + index);
    return;
  }

  const int start_file = index;

  const int size = static_cast<int>(kSpan.size()) - start_file;
  const std::span<const std::byte> src(kSpan.data() + start_file, size);

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
