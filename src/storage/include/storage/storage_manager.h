
#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <logging/logger.h>

#include <expected>
#include <filesystem>
#include <optional>
#include <span>
#include <string_view>
#include <system_error>
#include <vector>
namespace screen_controller {

class IStorageManager {
 public:
  virtual ~IStorageManager() = default;

  virtual std::expected<void, std::error_code> Init() const = 0;
  virtual std::optional<std::vector<std::byte>> LoadResource(
      std::string_view name) const = 0;
  virtual std::optional<std::vector<std::byte>> LoadFile(
      std::string_view name) const = 0;
  virtual bool SaveFile(std::string_view name,
                        std::span<std::byte> data) const = 0;
  virtual bool SaveFile(std::string_view name,
                        const std::vector<std::byte>& data) const = 0;
  virtual bool DeleteFile(std::string_view path) const = 0;
  virtual std::filesystem::path GetUserFilePath(
      std::string_view name) const = 0;
  virtual std::filesystem::path GetResourcePath(
      std::string_view name) const = 0;
};

class StorageFactory {
 public:
  static std::unique_ptr<IStorageManager> Create(ILogger& logger);
};

}  // namespace screen_controller

#endif
