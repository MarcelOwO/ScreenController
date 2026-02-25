#ifndef I_FILE_PROCESSOR_H
#define I_FILE_PROCESSOR_H

#include <models/frame_data.h>

#include <memory>
#include <optional>

namespace screen_controller {

class IFileProcessor {
 public:
  virtual ~IFileProcessor() = 0;

  virtual bool process_file(std::string_view path) = 0;

  virtual std::optional<std::unique_ptr<common::FrameData>> get_processed_data()
      const = 0;
};

class ProcessorFactory {
  static std::unique_ptr<IFileProcessor> Create();
};

}  // namespace screen_controller

#endif
