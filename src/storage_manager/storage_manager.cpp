//
// Created by marce on 4/23/2025.
//

#include "storage_manager.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>

namespace screen_controller {
StorageManager::StorageManager()
    : asset_path_("assets"), user_files_path_("files") {}

StorageManager::~StorageManager() =default;

bool StorageManager::Init() const {
  if (!std::filesystem::exists(user_files_path_)) {
    if (!std::filesystem::create_directory(user_files_path_)) {
      std::cerr << "Error creating directory " << user_files_path_ << std::endl;
      return false;
    }
  }

  if (!std::filesystem::exists(asset_path_)) {
    if (!std::filesystem::create_directory(asset_path_)) {
      std::cerr << "Error creating directory " << asset_path_ << std::endl;
      return false;
    }
  }

  return true;
}


std::optional<std::vector<std::byte>> StorageManager::LoadResource(
    std::string_view name) const {
  auto path = GetResourcePath(name);
  if (!std::filesystem::exists(path) ||
      !std::filesystem::is_regular_file(path)) {
    std::cerr << "Error: Resource file not found or is not a regular file: "
              << path.string() << std::endl;
    return std::nullopt;
  }

  std::ifstream file(path, std::ios::binary | std::ios::ate);

  if (!file.is_open()) {
    std::cerr << "Error opening file: " << name << std::endl;
    return std::nullopt;
  }

  std::streamsize size = file.tellg();
  if (size == -1) {
    std::cerr << "Error reading file: " << name << std::endl;
    return std::nullopt;
  }

  (void)file.seekg(0, std::ios::beg);

  std::vector<std::byte> buffer(static_cast<size_t>(size));
  if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
    std::cerr << "Error reading file: " << name << std::endl;
    return std::nullopt;
  }
  return buffer;
}
std::filesystem::path StorageManager::GetPath(std::string_view name) const {
  return GetResourcePath(name);
}

std::optional<std::vector<std::byte>> StorageManager::LoadFile(
    std::string_view name) const {
  const auto path = GetUserFilePath(name);
  if (!std::filesystem::exists(path) ||
      !std::filesystem::is_regular_file(path)) {
    std::cerr << "Error: User file not found or is not a regular file: "
              << path.string() << std::endl;
    return std::nullopt;
  }

  std::ifstream file(path, std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    std::cerr << "Error opening file: " << name << std::endl;
    return std::nullopt;
  }

  std::streamsize size = file.tellg();
  if (size == -1) {
    std::cerr << "Error reading file: " << name << std::endl;
    return std::nullopt;
  }

  (void)file.seekg(std::ios::beg);

  std::vector<std::byte> buffer(static_cast<size_t>(size));
  if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
    std::cerr << "Error reading file: " << name << std::endl;
    return std::nullopt;
  }
  return buffer;
}

bool StorageManager::SaveFile(std::string_view name,
                              const std::vector<std::byte>& data) {
  const auto path = GetUserFilePath(name);
  std::ofstream file(path, std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    std::cerr << "Error opening file: " << name << std::endl;
    return false;
  }

  if (!file.write(reinterpret_cast<const char*>(data.data()), data.size())) {
    std::cerr << "Error writing file: " << name << std::endl;
    file.close();
    if (!std::filesystem::remove(path)) {
      std::cerr << "Error removing file: " << name << std::endl;
    }
    return false;
  }
  return true;
}
bool StorageManager::DeleteFile(std::string_view name) {
  const auto path = GetUserFilePath(name);
  if (!std::filesystem::exists(path)) {
    return true;
  }

  if (!std::filesystem::remove(path)) {
    std::cerr << "Error removing file: " << name << std::endl;
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
