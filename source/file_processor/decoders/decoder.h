//
// Created by marce on 5/6/2025.
//

#ifndef PROCESSOR_INTERFACE_H
#define PROCESSOR_INTERFACE_H
#include <memory>
#include <optional>

#include "models/frame_data.h"

namespace screen_controller::processing {

class Decoder {
 public:
  virtual ~Decoder() = default;
  virtual bool init() = 0;
  virtual std::optional<std::unique_ptr<models::FrameData>>
  get_next_frame() = 0;
  virtual bool has_data() = 0;
};

}  // namespace screen_controller::processing

#endif  // PROCESSOR_INTERFACE_H
