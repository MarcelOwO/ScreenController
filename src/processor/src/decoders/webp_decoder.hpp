//
// Created by marce on 5/6/2025.
//
#pragma once

#include <models/frame_data.hpp>

#include <optional>
#include <string>

#include <logging/logger.hpp>
#include "decoder.hpp"

namespace screen_controller::processing {

class WebpDecoder final : public IDecoder {
public:
  explicit WebpDecoder(std::string_view path, ILogger& logger);
  ~WebpDecoder() override;
  WebpDecoder(const WebpDecoder&) = delete;
  WebpDecoder& operator=(const WebpDecoder&) = delete;
  WebpDecoder(WebpDecoder&&) = delete;
  WebpDecoder& operator=(WebpDecoder&&) = delete;

  bool Init() override;
  bool HasData() override;
  std::optional<std::unique_ptr<FrameData>> GetNextFrame() override;

private:
  ILogger& logger_;
  FrameData frame_data_;
  bool is_loaded_;
  std::string path_;
};

}  // namespace screen_controller::processing
