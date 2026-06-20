//
// Created by marce on 5/7/2025.
//

#pragma once

#include <memory>
#include <string_view>

#include "decoder.hpp"
#include <enums/file_type.hpp>
#include <logging/logger.hpp>

namespace screen_controller::processing {

class DecoderFactory {
 public:
  static std::unique_ptr<IDecoder> Create(std::string_view name, FileType type, ILogger& logger);
};

}  // namespace screen_controller::processing

