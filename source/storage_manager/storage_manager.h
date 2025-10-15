//
// Created by marce on 4/23/2025.
//

#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <filesystem>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace screen_controller {
class StorageManager {
 public:
  StorageManager();
  ~StorageManager();

  bool Init() const;
  std::optional<std::vector<std::byte>> LoadResource(
      std::string_view name) const;
  std::optional<std::vector<std::byte>> LoadFile(std::string_view name) const;

  bool SaveFile(std::string_view name, std::span<std::byte> data);
  bool SaveFile(std::string_view name, const std::vector<std::byte>& data);
  bool DeleteFile(std::string_view path);

  std::filesystem::path GetUserFilePath(std::string_view name) const;
  std::filesystem::path GetResourcePath(std::string_view name) const;
 private:
  std::filesystem::path asset_path_;
  std::filesystem::path user_files_path_;

};
}  // namespace screen_controller

#endif  // STORAGE_MANAGER_H
