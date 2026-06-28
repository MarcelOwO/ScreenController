#include <helper/unwrap.hpp>
#include <memory>
#include <processor/file_processor.hpp>

#include "file_processor_impl.hpp"

namespace screen_controller {

std::unique_ptr<IFileProcessor> ProcessorFactory::Create(ILogger& logger) {
  return Unwrap(FileProcessor::Create(logger), "Failed to create file processor");
}

}  // namespace screen_controller
