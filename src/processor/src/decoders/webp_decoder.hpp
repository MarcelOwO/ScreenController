//
// Created by marce on 5/6/2025.
//
#pragma once

#include <models/frame_data.hpp>

#include <optional>
#include <string_view>

#include <logging/logger.hpp>
#include "decoder.hpp"

namespace screen_controller::processing {

class WebpDecoder final : public IDecoder {
public:
  explicit WebpDecoder(std::string_view path, ILogger& logger);
  virtual ~WebpDecoder() override;
  WebpDecoder(const WebpDecoder&) = delete;
  WebpDecoder& operator=(const WebpDecoder&) = delete;
  WebpDecoder(WebpDecoder&&) = delete;
  WebpDecoder& operator=(WebpDecoder&&) = delete;

  virtual bool init() override;
  virtual bool has_data() override;
  virtual std::optional<std::unique_ptr<FrameData>> get_next_frame() override;

private:
  ILogger& logger_;
  FrameData frame_data_;
  bool is_loaded_;
  std::string_view path_;
};

}  // namespace screen_controller::processing
