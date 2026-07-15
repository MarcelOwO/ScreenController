
// Created by marce on 5/6/2025.
//

#include "stb_decoder.hpp"

#include <limits>
#include <optional>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "../../external/stb/stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "../../external/stb/stb_image_resize2.h"

namespace screen_controller::processing {

StbDecoder::StbDecoder(const std::string_view path, ILogger& logger)
    : logger_(logger), is_loaded_(false), path_(path) {
  logger_.LogFmt(LogLevel::INFO, "Creating StbDecoder for path: {}", path);
}

bool StbDecoder::HasData() {
  return is_loaded_;
}

bool StbDecoder::Init() {
  frame_data_ = {};

  auto* raw_data =
      stbi_load(path_.c_str(), &frame_data_.width, &frame_data_.height, &frame_data_.channels, 3);

  if (raw_data == nullptr) {
    logger_.LogFmt(LogLevel::ERROR, "Failed to load image: {}", path_);
    return false;
  }

  constexpr int kMaxDimension = 16384;
  if (frame_data_.width <= 0 || frame_data_.height <= 0 || frame_data_.width > kMaxDimension ||
      frame_data_.height > kMaxDimension) {
    stbi_image_free(raw_data);
    logger_.LogError("Image dimensions are invalid or too large");
    return false;
  }
  const std::size_t size = static_cast<std::size_t>(frame_data_.width) *
                           static_cast<std::size_t>(frame_data_.height) * 3U;
  frame_data_.data.assign(raw_data, raw_data + size);
  stbi_image_free(raw_data);
  frame_data_.channels = 3;
  is_loaded_ = true;
  return true;
}

std::optional<std::unique_ptr<FrameData>> StbDecoder::GetNextFrame() {
  if (!is_loaded_) {
    return std::nullopt;
  }
  is_loaded_ = false;
  return std::make_unique<FrameData>(frame_data_);
}

}  // namespace screen_controller::processing
