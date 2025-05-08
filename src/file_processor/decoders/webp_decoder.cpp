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
    : buffer_(std::vector<uint8_t>()), is_loaded_(false), path_(path) {}

WebpDecoder::~WebpDecoder() { buffer_.clear(); }

bool WebpDecoder::init() {
  std::ifstream file(path_.data(), std::ios::binary | std::ios::ate);

  if (!file) {
    std::cerr << "opening file failed" << "\n";
    return false;
  }

  const std::streamsize size = file.tellg();

  if (size == -1) {
    std::cerr << "Failed to read file" << std::endl;
    file.close();
    return false;
  }

  (void)file.seekg(0, std::ios::beg);

  std::vector<uint8_t> input_buffer(size);

  if (!file.read(reinterpret_cast<char *>(input_buffer.data()), size)) {
    std::cerr << "Failed to read file" << std::endl;
    file.close();
    return false;
  }

  file.close();

  WebPDecoderConfig config;
  if (WebPInitDecoderConfig(&config) < 0) {
    std::cerr << "WebPInitDecoderConfig failed " << "\n";
    return false;
  }

  if (auto result = WebPGetFeatures(input_buffer.data(), input_buffer.size(),
                                    &config.input);
      result != VP8_STATUS_OK) {
    std::cerr << "WebPGetFeatures failed with code: " << result << "\n";
    return false;
  }

  config.options.use_scaling = 1;
  config.options.scaled_width = 1920;
  config.options.scaled_height = 1080;

  constexpr size_t output_size = 1920 * 1080 * 3;

  buffer_.resize(output_size);
  config.output.colorspace = MODE_RGB;

  if (auto result =
          WebPDecode(input_buffer.data(), input_buffer.size(), &config);
      result != VP8_STATUS_OK) {
    std::cerr << "WebPDecode failed with code: " << result << "\n";
    return false;
  }

  const auto a = config.output.private_memory;
  buffer_ = std::vector<uint8_t>(a, output_size + a);

  WebPFreeDecBuffer(&config.output);

  return true;
}

std::optional<std::shared_ptr<models::FrameData>>
WebpDecoder::get_next_frame() {
  if (is_loaded_) {
    return std::nullopt;
  }

  models::FrameData image_data{
      .data = buffer_,
      .width = 1920,
      .height = 1080,
  };

  is_loaded_ = true;

  return std::make_shared<models::FrameData>(image_data);
}

}  // namespace screen_controller::processing