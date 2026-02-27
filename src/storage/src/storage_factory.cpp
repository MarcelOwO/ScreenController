#include <storage/storage_manager.h>

#include "storage_manager_impl.h"

namespace screen_controller {

std::unique_ptr<IStorageManager> StorageFactory::Create(ILogger& logger) {
  return std::make_unique<StorageManager>(logger);
}

}  // namespace screen_controller
