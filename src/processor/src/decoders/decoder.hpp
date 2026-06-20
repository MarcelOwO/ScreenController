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
  virtual bool Init() = 0;
  virtual std::optional<std::unique_ptr<FrameData>> GetNextFrame() = 0;
  virtual bool HasData() = 0;
};

}  // namespace screen_controller::processing
