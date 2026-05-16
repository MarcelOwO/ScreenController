//
// Created by marce on 5/6/2025.
//

#ifndef PROCESSOR_INTERFACE_H
#define PROCESSOR_INTERFACE_H
#include <models/frame_data.h>

#include <memory>
#include <optional>

<<<<<<<< HEAD:source/file_processor/decoders/decoder.h
#include <models/frame_data.h>

========
>>>>>>>> origin/dev:source/processor/src/decoders/decoder.h
namespace screen_controller::processing {

class IDecoder {
 public:
  virtual ~IDecoder() = default;
  virtual bool init() = 0;
  virtual std::optional<std::unique_ptr<common::FrameData>>
  get_next_frame() = 0;
  virtual bool has_data() = 0;
};

}  // namespace screen_controller::processing

#endif  // PROCESSOR_INTERFACE_H
