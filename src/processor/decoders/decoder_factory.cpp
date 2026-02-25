//
// Created by marce on 5/7/2025.
//

#include "decoder_factory.h"

#include <file_type.h>
#include <logging/logger.h>

#include <iostream>

#include "stb_decoder.h"
#include "video_decoder.h"
#include "webp_decoder.h"

namespace screen_controller::processing {
std::unique_ptr<IDecoder> DecoderFactory::create(std::string_view name,
                                                 const common::FileType type,
                                                 ILogger& logger) {
  switch (type) {
    case common::FileType::kBmp: {
      return std::make_unique<StbDecoder>(name, logger);
    }
    case common::FileType::kJpg: {
      return std::make_unique<VideoDecoder>(name, logger);
    }
    case common::FileType::kPng: {
      return std::make_unique<StbDecoder>(name, logger);
    }
    case common::FileType::kGif: {
      return std::make_unique<VideoDecoder>(name, logger);
    }
    case common::FileType::kWebp: {
      return std::make_unique<WebpDecoder>(name, logger);
    }
    case common::FileType::kWebm: {
      return std::make_unique<VideoDecoder>(name, logger);
    }
    case common::FileType::kMp4: {
      return std::make_unique<VideoDecoder>(name, logger);
    }
    case common::FileType::kNone: {
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
