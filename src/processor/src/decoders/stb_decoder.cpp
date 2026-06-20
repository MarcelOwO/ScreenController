
// Created by marce on 5/6/2025.
//

#include "stb_decoder.hpp"

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
  frame_data_.data.resize(1920 * 1080 * 3);

  auto* raw_data =
      stbi_load(path_.c_str(), &frame_data_.width, &frame_data_.height, &frame_data_.channels, 3);

  if (raw_data == nullptr) {
    logger_.LogFmt(LogLevel::ERROR, "Failed to load image: {}", path_);
    return false;
  }

  const auto* result =
      stbir_resize_uint8_srgb(raw_data, frame_data_.width, frame_data_.height, 0,
                               frame_data_.data.data(), 1920, 1080, 0, STBIR_RGB);
  stbi_image_free(raw_data);

  if (result == nullptr) {
    logger_.LogError("Failed to resize image");
    return false;
  }

  frame_data_.width = 1920;
  frame_data_.height = 1080;
  is_loaded_ = true;
  return true;
}

std::optional<std::unique_ptr<FrameData>> StbDecoder::GetNextFrame() {
  return std::make_unique<FrameData>(frame_data_);
}

}  // namespace screen_controller::processing
