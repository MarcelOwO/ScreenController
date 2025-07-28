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
#include "ng-log/logging.h"

namespace screen_controller::processing {

WebpDecoder::WebpDecoder(const std::string_view path)
    : is_loaded_(false), path_(path) {
  LOG(INFO) << "Created WebpDecoder";
}

WebpDecoder::~WebpDecoder() = default;

bool WebpDecoder::init() {
  std::ifstream file(path_.data(), std::ios::binary | std::ios::ate);

  CHECK(file.is_open()) << "Failed to open file: " << path_;

  const std::streamsize size = file.tellg();

  if (size <= 0) {
    LOG(ERROR) << "File is empty or could not be read: " << path_;
    file.close();
    return false;
  }

  CHECK(size > 0) << "File is empty or could not be read: " << path_;

  (void)file.seekg(0, std::ios::beg);

  std::vector<uint8_t> input_buffer(size);

  CHECK(file.read(reinterpret_cast<char *>(input_buffer.data()), size))
      << "Failed to read file: " << path_;

  file.close();

  WebPDecoderConfig config;

  CHECK(WebPInitDecoderConfig(&config) >= 0) << "WebPInitDecoderConfig failed";

  CHECK(WebPGetFeatures(input_buffer.data(), input_buffer.size(),
                        &config.input) == VP8_STATUS_OK)
      << "WebPGetFeatures failed";

  const int width = config.input.width;
  const int height = config.input.height;

  const size_t output_size = width * height * 3;

  config.output.colorspace = MODE_RGB;

  PCHECK(WebPDecode(input_buffer.data(), input_buffer.size(), &config) ==
         VP8_STATUS_OK)
      << "WebPDecode failed with code: ";

  const auto private_memory = config.output.private_memory;

  frame_data_ = {
      .data =
          std::vector<uint8_t>(private_memory, output_size + private_memory),
      .width = width,
      .height = height,
      .channels = 3,
  };
  is_loaded_ = true;
  WebPFreeDecBuffer(&config.output);

  return true;
}
bool WebpDecoder::has_data() { return is_loaded_; }

std::optional<std::unique_ptr<models::FrameData>>
WebpDecoder::get_next_frame() {
  return std::make_unique<models::FrameData>(frame_data_);
}

}  // namespace screen_controller::processing