#include <storage/storage_manager.hpp>

#include "storage_manager_impl.hpp"

namespace screen_controller {

std::unique_ptr<IStorageManager> StorageFactory::Create(ILogger& logger) {
  auto storage_manager = StorageManager::Create(logger);

  if (!storage_manager) {
    throw std::runtime_error("Failed to create storage manager");
  }

  return std::move(storage_manager.value());
}

}  // namespace screen_controller
