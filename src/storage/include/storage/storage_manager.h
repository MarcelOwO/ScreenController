
#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <logging/logger.h>

#include <filesystem>
#include <optional>
#include <span>
#include <string_view>
#include <vector>
namespace screen_controller {

class IStorageManager {
public:
  virtual ~IStorageManager() = default;

  [[nodiscard]] virtual std::optional<std::vector<std::byte>> LoadResource(
      std::string_view name) const = 0;
  [[nodiscard]] virtual std::optional<std::vector<std::byte>> LoadFile(
      std::string_view name) const = 0;
  [[nodiscard]] virtual bool SaveFile(std::string_view name, std::span<std::byte> data) const = 0;
  [[nodiscard]] virtual bool SaveFile(std::string_view name,
                                      const std::vector<std::byte>& data) const = 0;
  [[nodiscard]] virtual bool DeleteFile(std::string_view path) const = 0;
  [[nodiscard]] virtual std::filesystem::path GetUserFilePath(std::string_view name) const = 0;
  [[nodiscard]] virtual std::filesystem::path GetResourcePath(std::string_view name) const = 0;
};

class StorageFactory {
public:
  static std::unique_ptr<IStorageManager> Create(ILogger& logger);
};

}  // namespace screen_controller

#endif
