//
// Created by marce on 5/6/2025.
//

#pragma once

#include <optional>
#include <string>

#include <logging/logger.hpp>
#include <models/frame_data.hpp>
#include "decoder.hpp"

namespace screen_controller::processing {

class StbDecoder final : public IDecoder {
public:
  explicit StbDecoder(std::string_view path, ILogger& logger);
  ~StbDecoder() override = default;

  StbDecoder(const StbDecoder&) = delete;
  StbDecoder(StbDecoder&&) = delete;
  StbDecoder& operator=(const StbDecoder&) = delete;
  StbDecoder& operator=(StbDecoder&&) = delete;

  bool HasData() override;
  bool Init() override;
  std::optional<std::unique_ptr<FrameData>> GetNextFrame() override;

private:
  FrameData frame_data_;
  ILogger& logger_;
  bool is_loaded_;
  std::string path_;
};

}  // namespace screen_controller::processing
