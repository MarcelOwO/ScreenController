//
// Created by marce on 5/31/2025.
//

#include "unpacker.h"

#include <bits/ranges_util.h>

#include <iomanip>

#include "zstd.h"

namespace screen_controller {
Unpacker::Unpacker(const std::shared_ptr<Logger>& logger) : logger_(logger) {}
Unpacker::~Unpacker() = default;


void Unpacker::decompress(const std::span<std::byte> span,
                          common::BluetoothPacket& packet) const {
  std::vector<char> debug_string{};

  const auto type = static_cast<uint8_t>(span[static_cast<size_t>(0)]);

  debug_string.push_back(static_cast<char>(type));
  debug_string.push_back(' ');

  packet.type = type;
  const int len_string = static_cast<int>(span[static_cast<size_t>(1)]);

  debug_string.push_back(static_cast<char>(len_string));
  debug_string.push_back(' ');

  std::vector<char> name_vector(len_string);

  for (int i = 0; i < len_string; ++i) {
    name_vector[i] = static_cast<char>(span[i + 2]);
  }

  const auto name = std::string(name_vector.data(), name_vector.size());

  packet.name = name;
  logger_->LogInfo(name);
  if (type == 1) {
    logger_->LogInfo("command");
    return;
  }
  logger_->LogInfo("File");

  std::vector<std::byte> magic_number = {
      std::byte{0x28},
      std::byte{0xb5},
      std::byte{0x2f},
      std::byte{0xfd},
  };

  const auto search_subrange = std::ranges::search(span, magic_number);

  if (search_subrange.empty()) {
    logger_->LogInfo("Magic number not found in the span");
    return;
  }

  const auto index = std::distance(span.begin(), search_subrange.begin());

  if (index < 0 || index >= static_cast<int>(span.size())) {
    logger_->LogError("Invalid index for magic number:" + index);
    return;
  }

  const int start_file = index;

  const int size = static_cast<int>(span.size()) - start_file;
  const std::span<const std::byte> src(span.data() + start_file, size);

  const size_t decompressed_size =
      ZSTD_getFrameContentSize(src.data(), src.size());
  if (decompressed_size == ZSTD_CONTENTSIZE_ERROR) {
    logger_->LogError("Not a valid compressed frame");
    return;
  }

  if (decompressed_size == ZSTD_CONTENTSIZE_UNKNOWN) {
    logger_->LogError("Compressed frame is unkown");
    return;
  }

  packet.data = std::vector<std::byte>(decompressed_size);

  const size_t result = ZSTD_decompress(packet.data.data(), decompressed_size,
                                        src.data(), src.size());

  if (ZSTD_isError(result)) {
    logger_->LogError("Decompression failed" +
                      std::string(ZSTD_getErrorName(result)));
    return;
  }
  logger_->LogInfo("Decompressed successfully");
}
}  // namespace screen_controller
