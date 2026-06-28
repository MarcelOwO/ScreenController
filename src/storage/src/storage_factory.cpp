#include <helper/unwrap.hpp>
#include <storage/storage_manager.hpp>

#include "storage_manager_impl.hpp"

namespace screen_controller {

std::unique_ptr<IStorageManager> StorageFactory::Create(ILogger& logger) {
  return Unwrap(StorageManager::Create(logger), "Failed to create storage manager");
}

}  // namespace screen_controller
