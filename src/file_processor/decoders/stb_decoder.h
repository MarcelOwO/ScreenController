//
// Created by marce on 5/6/2025.
//

#ifndef STB_PROCESSOR_H
#define STB_PROCESSOR_H

#include <optional>

#include "decoder.h"
#include "models/frame_data.h"

namespace screen_controller::processing {

class StbDecoder final : public Decoder {
 public:
  explicit StbDecoder(std::string_view path);
  virtual ~StbDecoder() override = default;

  StbDecoder(const StbDecoder&) = delete;
  StbDecoder(StbDecoder&&) = delete;
  StbDecoder& operator=(const StbDecoder&) = delete;
  StbDecoder& operator=(StbDecoder&&) = delete;

  virtual std::optional<std::shared_ptr<models::FrameData>> get_next_frame() override;

 private:
  bool is_loaded_;
  std::string_view path_;
};

}  // namespace screen_controller::processing

#endif  // STB_PROCESSOR_H
