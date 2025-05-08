
// Created by marce on 5/6/2025.
//

#include "stb_decoder.h"

#include <iostream>
#include <optional>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "../../external/stb/stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "../../external/stb/stb_image_resize2.h"

namespace screen_controller::processing {

StbDecoder::StbDecoder(const std::string_view path)
    : decoded_frames_(std::vector<uint8_t>(1920 * 1080 * 3)),
      is_loaded_(false),
      path_(path) {}

bool StbDecoder::init() {
  models::FrameData image_data{};

  const auto raw_data = stbi_load(path_.data(), &image_data.width,
                                  &image_data.height, &image_data.channels, 3);

  if (!raw_data) {
    std::cerr << "FileProcessor::process_file: failed to load image"
              << std::endl;
    return false;
  }

  (void)stbir_resize_uint8_srgb(raw_data, image_data.width, image_data.height,
                                0, decoded_frames_.data(), 1920, 1080, 0,
                                STBIR_RGB);

  if (decoded_frames_.empty()) {
    std::cerr << "FileProcessor::process_file: failed to resize image"
              << std::endl;
    return false;
  }

  stbi_image_free(raw_data);

  return true;
}

std::optional<std::shared_ptr<models::FrameData>> StbDecoder::get_next_frame() {
  if (is_loaded_) {
    return std::nullopt;
  }
  models::FrameData image_data{
      .data = decoded_frames_,
  };
  is_loaded_ = true;
  return std::make_shared<models::FrameData>(image_data);
}

}  // namespace screen_controller::processing