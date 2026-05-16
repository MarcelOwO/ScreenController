
// Created by marce on 5/6/2025.
//

#include "stb_decoder.h"

<<<<<<<< HEAD:source/file_processor/decoders/stb_decoder.cpp
#include <iostream>
========
>>>>>>>> origin/dev:source/processor/src/decoders/stb_decoder.cpp
#include <optional>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "../../external/stb/stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "../../external/stb/stb_image_resize2.h"

namespace screen_controller::processing {

<<<<<<<< HEAD:source/file_processor/decoders/stb_decoder.cpp
StbDecoder::StbDecoder(const std::string_view path,
                       const std::shared_ptr<Logger> &logger)
    : logger_(logger), is_loaded_(false), path_(path) {
  logger_->LogInfo("Creating StbDecoder for path: " + std::string(path));
========
StbDecoder::StbDecoder(const std::string_view path, ILogger& logger)
    : logger_(logger), is_loaded_(false), path_(path) {
  logger_.LogInfo("Creating StbDecoder for path: " + std::string(path));
>>>>>>>> origin/dev:source/processor/src/decoders/stb_decoder.cpp
}

bool StbDecoder::has_data() { return is_loaded_; }
bool StbDecoder::init() {
  frame_data_ = {};
  frame_data_.data.resize(1920 * 1080 * 3);

  const auto raw_data =
      stbi_load(path_.data(), &frame_data_.width, &frame_data_.height,
                &frame_data_.channels, 3);

  if (raw_data == nullptr) {
<<<<<<<< HEAD:source/file_processor/decoders/stb_decoder.cpp
    logger_->LogError("Failed to load image");
========
    logger_.LogError("Failed to load image");
>>>>>>>> origin/dev:source/processor/src/decoders/stb_decoder.cpp
  }

  (void)stbir_resize_uint8_srgb(raw_data, frame_data_.width, frame_data_.height,
                                0, frame_data_.data.data(), 1920, 1080, 0,
                                STBIR_RGB);

  if (!!frame_data_.data.empty()) {
<<<<<<<< HEAD:source/file_processor/decoders/stb_decoder.cpp
    logger_->LogError("Failed to resize image");
========
    logger_.LogError("Failed to resize image");
>>>>>>>> origin/dev:source/processor/src/decoders/stb_decoder.cpp
  }
  is_loaded_ = true;
  stbi_image_free(raw_data);
  return true;
}

std::optional<std::unique_ptr<common::FrameData>> StbDecoder::get_next_frame() {
  return std::make_unique<common::FrameData>(frame_data_);
}

}  // namespace screen_controller::processing
