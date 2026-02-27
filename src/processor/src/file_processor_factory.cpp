#include <processor/file_processor.h>

#include <memory>

#include "file_processor_impl.h"

namespace screen_controller {

std::unique_ptr<IFileProcessor> ProcessorFactory::Create(ILogger& logger) {
  return std::make_unique<FileProcessor>(logger);
}

}  // namespace screen_controller
