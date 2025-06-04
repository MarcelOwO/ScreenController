
// Created by marce on 5/6/2025.
//

#include "stb_decoder.h"

#include <ng-log/logging.h>

#include <iostream>
#include <optional>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "../../external/stb/stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "../../external/stb/stb_image_resize2.h"

namespace screen_controller::processing {

StbDecoder::StbDecoder(const std::string_view path)
    : is_loaded_(false), path_(path) {
  LOG(INFO) << "Initialized STB-Decoder";
}

bool StbDecoder::has_data() {
  return is_loaded_;
}
bool StbDecoder::init() {
  frame_data_ = {};
  frame_data_.data.resize(1920 * 1080 * 3);

  const auto raw_data =
      stbi_load(path_.data(), &frame_data_.width, &frame_data_.height,
                &frame_data_.channels, 3);

  PCHECK(raw_data != nullptr) << "Failed to load image";


  (void)stbir_resize_uint8_srgb(raw_data, frame_data_.width, frame_data_.height,
                                0, frame_data_.data.data(), 1920, 1080, 0,
                                STBIR_RGB);

  PCHECK(!frame_data_.data.empty()) << "Failed to resize image";
  is_loaded_ = true;
  stbi_image_free(raw_data);
  return true;
}

std::optional<std::unique_ptr<models::FrameData>> StbDecoder::get_next_frame() {
  return std::make_unique<models::FrameData>(frame_data_);
}

}  // namespace screen_controller::processing