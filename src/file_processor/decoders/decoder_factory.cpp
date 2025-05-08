//
// Created by marce on 5/7/2025.
//

#include "decoder_factory.h"

#include <iostream>

#include "../../common/file_type.h"
#include "stb_decoder.h"
#include "video_decoder.h"
#include "webp_decoder.h"

namespace screen_controller::processing {
std::unique_ptr<processing::Decoder> DecoderFactory::create(
    std::string_view name, common::FileType type) {
  switch (type) {
    case common::FileType::kBmp: {
      return std::make_unique<processing::StbDecoder>(name);
    }
    case common::FileType::kJpg: {
      return std::make_unique<processing::VideoDecoder>(name);
    }
    case common::FileType::kPng: {
      return std::make_unique<processing::StbDecoder>(name);
    }
    case common::FileType::kGif: {
      return std::make_unique<processing::VideoDecoder>(name);
    }
    case common::FileType::kWebp: {
      return std::make_unique<processing::WebpDecoder>(name);
    }
    case common::FileType::kWebm: {
      return std::make_unique<processing::VideoDecoder>(name);
    }
    case common::FileType::kMp4: {
      return std::make_unique<processing::VideoDecoder>(name);
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