//
// Created by marce on 5/6/2025.
//

#pragma once

#include <optional>

#include <logging/logger.hpp>
#include <models/frame_data.hpp>
#include "decoder.hpp"

namespace screen_controller::processing {

class StbDecoder final : public IDecoder {
public:
  explicit StbDecoder(std::string_view path, ILogger& logger);
  virtual ~StbDecoder() override = default;

  StbDecoder(const StbDecoder&) = delete;
  StbDecoder(StbDecoder&&) = delete;
  StbDecoder& operator=(const StbDecoder&) = delete;
  StbDecoder& operator=(StbDecoder&&) = delete;
  virtual bool has_data() override;

  virtual bool init() override;
  virtual std::optional<std::unique_ptr<FrameData>> get_next_frame() override;

private:
  FrameData frame_data_;
  ILogger& logger_;
  bool is_loaded_;
  std::string_view path_;
};

}  // namespace screen_controller::processing
