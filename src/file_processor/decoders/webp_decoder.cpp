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

  PCHECK(file.is_open()) << "Failed to open file: " << path_;

  const std::streamsize size = file.tellg();

  PCHECK(size > 0) << "File is empty or could not be read: " << path_;

  (void)file.seekg(0, std::ios::beg);

  std::vector<uint8_t> input_buffer(size);

  PCHECK(file.read(reinterpret_cast<char *>(input_buffer.data()), size))
      << "Failed to read file: " << path_;

  file.close();

  WebPDecoderConfig config;

  PCHECK(WebPInitDecoderConfig(&config) >= 0) << "WebPInitDecoderConfig failed";

  PCHECK(WebPGetFeatures(input_buffer.data(), input_buffer.size(),
                         &config.input) == VP8_STATUS_OK)
      << "WebPGetFeatures failed";

  config.options.use_scaling = 1;
  config.options.scaled_width = 1920;
  config.options.scaled_height = 1080;

  constexpr size_t output_size = 1920 * 1080 * 3;

  config.output.colorspace = MODE_RGB;

  PCHECK(WebPDecode(input_buffer.data(), input_buffer.size(), &config) ==
         VP8_STATUS_OK)
      << "WebPDecode failed with code: ";

  const auto a = config.output.private_memory;

  frame_data_ = {
      .data = std::vector<uint8_t>(a, output_size + a),
      .width = config.output.width,
      .height = config.output.height,
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