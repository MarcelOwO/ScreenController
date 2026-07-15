//
// Created by marce on 5/31/2025.
//

#include "unpacker.hpp"

#include <cstddef>
#include <string>

#include "zstd.h"

namespace screen_controller::bluetooth {
std::expected<std::vector<std::byte>, std::string> Unpacker::Decompress(
    const std::span<const std::byte> compressed, const std::size_t max_output_size) const {
  if (compressed.empty()) {
    return std::unexpected("compressed payload is empty");
  }

  const size_t decompressed_size = ZSTD_getFrameContentSize(compressed.data(), compressed.size());
  if (decompressed_size == ZSTD_CONTENTSIZE_ERROR) {
    return std::unexpected("payload is not a valid zstd frame");
  }

  if (decompressed_size == ZSTD_CONTENTSIZE_UNKNOWN) {
    return std::unexpected("zstd frame must declare its decompressed size");
  }

  if (decompressed_size > max_output_size) {
    return std::unexpected("decompressed file exceeds the configured size limit");
  }

  std::vector<std::byte> output(decompressed_size);

  const size_t result =
      ZSTD_decompress(output.data(), output.size(), compressed.data(), compressed.size());

  if (ZSTD_isError(result)) {
    return std::unexpected("zstd decompression failed: " + std::string(ZSTD_getErrorName(result)));
  }
  if (result != decompressed_size) {
    return std::unexpected("zstd output size did not match its frame header");
  }
  return output;
}
}  // namespace screen_controller::bluetooth
