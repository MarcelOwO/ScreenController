//
// Created by marce on 5/6/2025.
//

#include "webp_decoder.hpp"

#include <webp/decode.h>
#include <models/frame_data.hpp>

#include <fstream>
#include <optional>
#include <string_view>

namespace screen_controller::processing {

WebpDecoder::WebpDecoder(const std::string_view path, ILogger& logger)
    : logger_(logger), is_loaded_(false), path_(path) {
  logger_.LogInfo("Created WebpDecoder");
}

WebpDecoder::~WebpDecoder() = default;

bool WebpDecoder::init() {
  std::ifstream file(path_.data(), std::ios::binary | std::ios::ate);

  if (!file.is_open()) {
    logger_.LogError("Failed to open file: " + std::string(path_));
  }

  const std::streamsize size = file.tellg();

  if (size <= 0) {
    logger_.LogError("File is empty or could not be read: " + std::string(path_));
    file.close();
    return false;
  }

  if (size <= 0) {
    logger_.LogError("File is empty or could not be read: " + std::string(path_));
  }

  (void) file.seekg(0, std::ios::beg);

  std::vector<uint8_t> input_buffer(size);

  if (!file.read(reinterpret_cast<char*>(input_buffer.data()), size)) {
    logger_.LogError(" Failed to read file: " + std::string(path_));
  }

  file.close();

  WebPDecoderConfig config;

  if (WebPInitDecoderConfig(&config) < 0) {
    logger_.LogError("WebPInitDecoderConfig failed");
  }

  if (WebPGetFeatures(input_buffer.data(), input_buffer.size(), &config.input) != VP8_STATUS_OK) {
    logger_.LogError("WebPGetFeatures failed");
  }

  const int width = config.input.width;
  const int height = config.input.height;

  const size_t output_size = width * height * 3;

  config.output.colorspace = MODE_RGB;

  if (WebPDecode(input_buffer.data(), input_buffer.size(), &config) != VP8_STATUS_OK) {
    logger_.LogError("WebPDecode failed with code: ");
  }

  const auto private_memory = config.output.private_memory;

  frame_data_ = {
      .data = std::vector<uint8_t>(private_memory, output_size + private_memory),
      .width = width,
      .height = height,
      .channels = 3,
  };
  is_loaded_ = true;
  WebPFreeDecBuffer(&config.output);

  return true;
}

bool WebpDecoder::has_data() {
  return is_loaded_;
}

std::optional<std::unique_ptr<FrameData>> WebpDecoder::get_next_frame() {
  return std::make_unique<FrameData>(frame_data_);
}

}  // namespace screen_controller::processing
