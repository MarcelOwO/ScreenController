//
// Created by marce on 4/23/2025.
//

#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <logging/logger.h>

#include <expected>
#include <filesystem>
#include <optional>
#include <span>
#include <vector>

namespace screen_controller {
class StorageManager {
 public:
  StorageManager(ILogger& logger);
  ~StorageManager();
  StorageManager(const StorageManager&) = delete;
  StorageManager& operator=(const StorageManager&) = delete;
  StorageManager(StorageManager&&) = delete;
  StorageManager& operator=(StorageManager&&) = delete;

  [[nodiscard]] std::expected<void, std::error_code> Init() const;

  std::optional<std::vector<std::byte>> LoadResource(
      std::string_view name) const;

  std::optional<std::vector<std::byte>> LoadFile(std::string_view name) const;

  bool SaveFile(std::string_view name, std::span<std::byte> data) const;
  bool SaveFile(std::string_view name,
                const std::vector<std::byte>& data) const;
  bool DeleteFile(std::string_view path) const;

  std::filesystem::path GetUserFilePath(std::string_view name) const;
  std::filesystem::path GetResourcePath(std::string_view name) const;

 private:
  ILogger& logger_;
  std::filesystem::path asset_path_;
  std::filesystem::path user_files_path_;
};
}  // namespace screen_controller

#endif  // STORAGE_MANAGER_H
