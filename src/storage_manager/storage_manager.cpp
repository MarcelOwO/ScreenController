//
// Created by marce on 4/23/2025.
//

#include "storage_manager.h"

#include <ng-log/logging.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>

namespace screen_controller {
StorageManager::StorageManager()
    : asset_path_("assets"), user_files_path_("files") {
  LOG(INFO) << "Creating StorageManager";
}

StorageManager::~StorageManager() = default;

bool StorageManager::Init() const {
  if (!std::filesystem::exists(user_files_path_)) {
    PCHECK(std::filesystem::create_directory(user_files_path_))
        << "Error creating directory " << user_files_path_;
  }

  if (!std::filesystem::exists(asset_path_)) {
    PCHECK(std::filesystem::create_directory(asset_path_))
        << "Error creating directory " << asset_path_;
  }

  return true;
}

std::optional<std::vector<std::byte>> StorageManager::LoadResource(
    std::string_view name) const {
  auto path = GetResourcePath(name);

  if (!std::filesystem::exists(path)) {
    PLOG(ERROR) << "Resource path does not exist: " << path.string();
    return std::nullopt;
  }
  if (!std::filesystem::is_regular_file(path)) {
    PLOG(ERROR) << "Resource path is not a regular file: " << path.string();
    return std::nullopt;
  }

  std::ifstream file(path, std::ios::binary | std::ios::ate);

  if (!file.is_open()) {
    PLOG(ERROR) << "Failed to open file: " << path.string();
    return std::nullopt;
  }

  std::streamsize size = file.tellg();
  if (size < 0) {
    PLOG(ERROR) << "Error reading file: " << name;
    return std::nullopt;
  }

  (void)file.seekg(0, std::ios::beg);

  std::vector<std::byte> buffer(static_cast<size_t>(size));
  if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
    PLOG(ERROR) << "Error reading file: " << name;
    return std::nullopt;
  }
  return buffer;
}

std::optional<std::vector<std::byte>> StorageManager::LoadFile(
    std::string_view name) const {
  const auto path = GetUserFilePath(name);

  if (!std::filesystem::exists(path)) {
    PLOG(ERROR) << "File path does not exist: " << path.string();
    return std::nullopt;
  }
  if (!std::filesystem::is_regular_file(path)) {
    PLOG(ERROR) << "Resource path does not exist: " << path.string();
    return std::nullopt;
  }

  std::ifstream file(path, std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    PLOG(ERROR) << "Error opening file: " << name;
    return std::nullopt;
  }

  std::streamsize size = file.tellg();
  if (size < 0) {
    PLOG(ERROR) << "Error reading file: " << name;
    return std::nullopt;
  }

  (void)file.seekg(std::ios::beg);

  std::vector<std::byte> buffer(static_cast<size_t>(size));
  if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
    PLOG(ERROR) << "Error reading file: " << name;
    return std::nullopt;
  }
  return buffer;
}
bool StorageManager::SaveFile(std::string_view name,
                              std::span<std::byte> data) {
  const auto path = GetUserFilePath(name);
  std::ofstream file(path, std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    PLOG(ERROR) << "Error opening file: " << name;
    return false;
  }

  if (!file.write(reinterpret_cast<const char*>(data.data()), data.size())) {
    PLOG(ERROR) << "Error writing file: " << name;
    file.close();
    if (!std::filesystem::remove(path)) {
      PLOG(ERROR) << "Error removing file: " << name;
    }
    return false;
  }
  PLOG(INFO) << "File saved successfully: " << name;
  return true;
}

bool StorageManager::SaveFile(std::string_view name,
                              const std::vector<std::byte>& data) {
  const auto path = GetUserFilePath(name);
  std::ofstream file(path, std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    PLOG(ERROR) << "Error opening file: " << name;
    return false;
  }

  if (!file.write(reinterpret_cast<const char*>(data.data()), data.size())) {
    PLOG(ERROR) << "Error writing file: " << name;
    file.close();
    if (!std::filesystem::remove(path)) {
      PLOG(ERROR) << "Error removing file: " << name;
    }
    return false;
  }
  return true;
}
bool StorageManager::DeleteFile(std::string_view name) {
  const auto path = GetUserFilePath(name);
  if (!std::filesystem::exists(path)) {
    PLOG(ERROR) << "Could not find file for deletion" << path.string();
    return true;
  }

  if (!std::filesystem::remove(path)) {
    PLOG(ERROR) << "Error removing file: " << name;
    return false;
  }
  return true;
}
std::filesystem::path StorageManager::GetUserFilePath(
    const std::string_view name) const {
  return user_files_path_ / name;
}
std::filesystem::path StorageManager::GetResourcePath(
    const std::string_view name) const {
  return asset_path_ / name;
}
}  // namespace screen_controller
