#pragma once

#include <logging/logger.hpp>
#include <models/frame_data.hpp>

#include <memory>
#include <optional>

namespace screen_controller {

class IFileProcessor {
public:
  virtual ~IFileProcessor() = default;

  virtual bool ProcessFile(std::string_view path) = 0;

  virtual std::optional<std::unique_ptr<FrameData>> GetProcessedData() const = 0;
};

class ProcessorFactory {
public:
  static std::unique_ptr<IFileProcessor> Create(ILogger& logger);
};

}  // namespace screen_controller
