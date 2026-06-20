//
// Created by marce on 4/23/2025.
//

#pragma once

#include <logging/logger.hpp>
#include <storage/storage_manager.hpp>

#include <expected>
#include <filesystem>
#include <optional>
#include <span>
#include <vector>

namespace screen_controller {
class StorageManager : public IStorageManager {
public:
  ~StorageManager();

  static std::expected<std::unique_ptr<StorageManager>, std::error_code> Create(ILogger& logger);

  StorageManager(const StorageManager&) = delete;
  StorageManager& operator=(const StorageManager&) = delete;
  StorageManager(StorageManager&&) = delete;
  StorageManager& operator=(StorageManager&&) = delete;

  std::optional<std::vector<std::byte>> LoadResource(std::string_view name) const;

  std::optional<std::vector<std::byte>> LoadFile(std::string_view name) const;

  bool SaveFile(std::string_view name, std::span<std::byte> data) const;
  bool SaveFile(std::string_view name, const std::vector<std::byte>& data) const;
  bool DeleteFile(std::string_view path) const;

  std::filesystem::path GetUserFilePath(std::string_view name) const;
  std::filesystem::path GetResourcePath(std::string_view name) const;

private:
  StorageManager(ILogger& logger);
  [[nodiscard]] std::expected<void, std::error_code> Init() const;

  ILogger& logger_;
  std::filesystem::path asset_path_;
  std::filesystem::path user_files_path_;
};
}  // namespace screen_controller
