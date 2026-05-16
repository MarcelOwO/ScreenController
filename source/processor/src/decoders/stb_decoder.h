//
// Created by marce on 5/6/2025.
//

#ifndef STB_PROCESSOR_H
#define STB_PROCESSOR_H

#include <optional>

#include "decoder.h"
#include "logging/logger.h"
#include "models/frame_data.h"

namespace screen_controller::processing {

class StbDecoder final : public IDecoder {
 public:
<<<<<<<< HEAD:source/file_processor/decoders/stb_decoder.h
  explicit StbDecoder(std::string_view path,
                      const std::shared_ptr<Logger>& logger);
========
  explicit StbDecoder(std::string_view path, ILogger& logger);
>>>>>>>> origin/dev:source/processor/src/decoders/stb_decoder.h
  virtual ~StbDecoder() override = default;

  StbDecoder(const StbDecoder&) = delete;
  StbDecoder(StbDecoder&&) = delete;
  StbDecoder& operator=(const StbDecoder&) = delete;
  StbDecoder& operator=(StbDecoder&&) = delete;
  virtual bool has_data() override;

  virtual bool init() override;
  virtual std::optional<std::unique_ptr<common::FrameData>> get_next_frame()
      override;

 private:
  common::FrameData frame_data_;
<<<<<<<< HEAD:source/file_processor/decoders/stb_decoder.h
  std::shared_ptr<Logger> logger_;
========
  ILogger& logger_;
>>>>>>>> origin/dev:source/processor/src/decoders/stb_decoder.h
  bool is_loaded_;
  std::string_view path_;
};

}  // namespace screen_controller::processing

#endif  // STB_PROCESSOR_H
