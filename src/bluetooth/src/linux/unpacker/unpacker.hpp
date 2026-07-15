//
// Created by marce on 5/31/2025.
//

#pragma once

#include <expected>
#include <span>
#include <string>
#include <vector>

namespace screen_controller::bluetooth {

class Unpacker final {
public:
  Unpacker() = default;
  ~Unpacker() = default;

  [[nodiscard]] std::expected<std::vector<std::byte>, std::string> Decompress(
      std::span<const std::byte> compressed, std::size_t max_output_size) const;

  Unpacker(const Unpacker&) = delete;
  Unpacker& operator=(const Unpacker&) = delete;
  Unpacker(Unpacker&&) = delete;
  Unpacker& operator=(Unpacker&&) = delete;
};

}  // namespace screen_controller::bluetooth
