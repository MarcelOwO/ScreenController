//
// Created by marce on 5/6/2025.
//

#include "webp_decoder.h"

#include <webp/decode.h>

#include <fstream>
#include <iostream>
#include <optional>
#include <string_view>

#include "models/frame_data.h"

namespace screen_controller::processing {

WebpDecoder::WebpDecoder(const std::string_view path)
    : is_loaded_(false), path_(path) {}

std::optional<std::shared_ptr<models::FrameData>>
WebpDecoder::get_next_frame() {
  if (is_loaded_) {
    return std::nullopt;
  }

  models::FrameData image_data{};

  std::ifstream file(path_.data(), std::ios::binary | std::ios::ate);

  if (!file) {
    std::cerr << "Failed to open file" << "\n";
    return std::nullopt;
  }

  const std::streamsize size = file.tellg();

  if (size == -1) {
    std::cerr << "Failed to read file" << std::endl;
    return std::nullopt;
  }

  (void)file.seekg(0, std::ios::beg);

  std::vector<uint8_t> buffer(size);

  if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
    std::cerr << "Failed to read file" << std::endl;
    return std::nullopt;
  }

  WebPDecoderConfig config;
  config.options.scaled_height = 1080;
  config.options.scaled_width = 1920;
  config.options.no_fancy_upsampling = 1;
  config.options.use_threads = 1;
  config.output.colorspace = MODE_RGB;
  config.output.u.RGBA.rgba = image_data.data.data();
  config.output.is_external_memory = 1;

  if (WebPDecode(buffer.data(), buffer.size(), &config) != VP8_STATUS_OK) {
    std::cerr << "Failed to decode file: " << std::endl;
    return std::nullopt;
  }

  if (image_data.data.empty()) {
    std::cerr << "Failed to decode file: " << std::endl;
    return std::nullopt;
  }
  is_loaded_ = true;
  return std::make_shared<models::FrameData>(image_data);
}

}  // namespace screen_controller::processing