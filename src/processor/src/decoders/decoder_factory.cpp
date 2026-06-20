//
// Created by marce on 5/7/2025.
//

#include "decoder_factory.hpp"

#include <enums/file_type.hpp>
#include <logging/logger.hpp>

#include <iostream>

#include "stb_decoder.hpp"
#include "video_decoder.hpp"
#include "webp_decoder.hpp"

namespace screen_controller::processing {
std::unique_ptr<IDecoder> DecoderFactory::create(std::string_view name, const FileType type,
                                                 ILogger& logger) {
  switch (type) {
    case FileType::kBmp: {
      return std::make_unique<StbDecoder>(name, logger);
    }
    case FileType::kJpg: {
      return std::make_unique<VideoDecoder>(name, logger);
    }
    case FileType::kPng: {
      return std::make_unique<StbDecoder>(name, logger);
    }
    case FileType::kGif: {
      return std::make_unique<VideoDecoder>(name, logger);
    }
    case FileType::kWebp: {
      return std::make_unique<WebpDecoder>(name, logger);
    }
    case FileType::kWebm: {
      return std::make_unique<VideoDecoder>(name, logger);
    }
    case FileType::kMp4: {
      return std::make_unique<VideoDecoder>(name, logger);
    }
    case FileType::kNone: {
      std::cerr << "DecoderFactory::create: unknown file type" << std::endl;
      return nullptr;
    }
    default: {
      std::cerr << "DecoderFactory::create: unknown file type" << std::endl;
      return nullptr;
    }
  }
  return nullptr;
}
}  // namespace screen_controller::processing
