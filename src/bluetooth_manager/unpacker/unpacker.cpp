//
// Created by marce on 5/31/2025.
//

#include "unpacker.h"

#include <bits/ranges_util.h>

#include <iomanip>

#include "ng-log/logging.h"
#include "zstd.h"

namespace screen_controller::bluetooth {
Unpacker::Unpacker() = default;
Unpacker::~Unpacker() = default;

void Unpacker::init() {}

void Unpacker::decompress(const std::span<std::byte> span,
                          common::BluetoothPacket& packet) {
  std::vector<char> debug_string{};

  const uint8_t type = static_cast<uint8_t>(span[static_cast<size_t>(0)]);

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

  LOG(INFO) << name;
  if (type == 1) {
    LOG(INFO) << "Command";
    return;
  }
  LOG(INFO) << "File";

  std::vector<std::byte> magic_number = {
      std::byte{0x28},
      std::byte{0xb5},
      std::byte{0x2f},
      std::byte{0xfd},
  };

  const auto search_subrange = std::ranges::search(span, magic_number);

  if (search_subrange.empty()) {
    LOG(INFO) << "Magic number not found in the span";
  }

  const auto index = std::distance(span.begin(), search_subrange.begin());
  if (index < 0 || index >= static_cast<int>(span.size())) {
    LOG(ERROR) << "Invalid index for magic number: " << index;
    return;
  }

  const int start_file = index;
  const int size = static_cast<int>(span.size()) - start_file;
  const std::span<const std::byte> src(span.data() + start_file, size);

  const size_t decompressed_size =
      ZSTD_getFrameContentSize(src.data(), src.size());
  if (decompressed_size == ZSTD_CONTENTSIZE_ERROR) {
    LOG(ERROR) << "Not a valid compressed frame";
    return;
  }
  if (decompressed_size == ZSTD_CONTENTSIZE_UNKNOWN) {
    LOG(ERROR) << "Compressed size is unknown";
    return;
  }

  packet.data = std::vector<std::byte>(decompressed_size);
  const size_t result = ZSTD_decompress(packet.data.data(), decompressed_size,
                                        src.data(), src.size());

  if (ZSTD_isError(result)) {
    LOG(ERROR) << "Decompression failed: " << ZSTD_getErrorName(result);
    return;
  }
  LOG(INFO) << "Decompressed successfully";
}

}  // namespace screen_controller::bluetooth