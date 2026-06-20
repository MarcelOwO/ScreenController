//
// Created by marce on 5/6/2025.
//
#pragma once

#include <models/frame_data.hpp>

#include <memory>
#include <optional>

namespace screen_controller::processing {

class IDecoder {
public:
  virtual ~IDecoder() = default;
  virtual bool init() = 0;
  virtual std::optional<std::unique_ptr<FrameData>> get_next_frame() = 0;
  virtual bool has_data() = 0;
};

}  // namespace screen_controller::processing
