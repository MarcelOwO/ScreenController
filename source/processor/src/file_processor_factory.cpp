#include <processor/file_processor.h>

#include <memory>

#include "file_processor_impl.h"

namespace screen_controller {

std::unique_ptr<IFileProcessor> ProcessorFactory::Create(ILogger& logger) {
  auto file_processor = FileProcessor::create(logger);
  if (!file_processor) {
    throw std::runtime_error("failed to create file processor");
  }
  return std::move(file_processor.value());
}

}  // namespace screen_controller
