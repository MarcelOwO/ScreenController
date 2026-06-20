//
// Created by marce on 5/6/2025.
//

#include "webp_decoder.hpp"

#include <webp/decode.h>
#include <models/frame_data.hpp>

#include <fstream>
#include <optional>

namespace screen_controller::processing {

WebpDecoder::WebpDecoder(const std::string_view path, ILogger& logger)
    : logger_(logger), is_loaded_(false), path_(path) {
  logger_.LogFmt(LogLevel::INFO, "Created WebpDecoder for: {}", path);
}

WebpDecoder::~WebpDecoder() = default;

bool WebpDecoder::Init() {
  std::ifstream file(path_, std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    logger_.LogFmt(LogLevel::ERROR, "Failed to open file: {}", path_);
    return false;
  }

  const std::streamsize size = file.tellg();
  if (size <= 0) {
    logger_.LogFmt(LogLevel::ERROR, "File is empty or unreadable: {}", path_);
    return false;
  }

  file.seekg(0, std::ios::beg);

  std::vector<uint8_t> input_buffer(static_cast<size_t>(size));
  if (!file.read(reinterpret_cast<char*>(input_buffer.data()), size)) {
    logger_.LogFmt(LogLevel::ERROR, "Failed to read file: {}", path_);
    return false;
  }

  file.close();

  WebPDecoderConfig config;
  if (!WebPInitDecoderConfig(&config)) {
    logger_.LogError("WebPInitDecoderConfig failed");
    return false;
  }

  if (WebPGetFeatures(input_buffer.data(), input_buffer.size(), &config.input) != VP8_STATUS_OK) {
    logger_.LogError("WebPGetFeatures failed");
    return false;
  }

  const int width = config.input.width;
  const int height = config.input.height;
  const size_t output_size = static_cast<size_t>(width * height * 3);

  config.output.colorspace = MODE_RGB;

  if (WebPDecode(input_buffer.data(), input_buffer.size(), &config) != VP8_STATUS_OK) {
    logger_.LogError("WebPDecode failed");
    return false;
  }

  const auto* pixel_data = config.output.u.RGBA.rgba;
  frame_data_ = {
      .data = std::vector<uint8_t>(pixel_data, pixel_data + output_size),
      .width = width,
      .height = height,
      .channels = 3,
  };

  WebPFreeDecBuffer(&config.output);

  is_loaded_ = true;
  return true;
}

bool WebpDecoder::HasData() {
  return is_loaded_;
}

std::optional<std::unique_ptr<FrameData>> WebpDecoder::GetNextFrame() {
  return std::make_unique<FrameData>(frame_data_);
}

}  // namespace screen_controller::processing
