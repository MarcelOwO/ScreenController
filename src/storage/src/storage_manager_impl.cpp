//
// Created by marce on 4/23/2025.
//

#include "storage_manager_impl.hpp"

#include <filesystem>
#include <fstream>
#include <optional>

namespace screen_controller {

StorageManager::StorageManager(ILogger& logger)
    : logger_(logger), asset_path_("assets"), user_files_path_("files") {}

std::expected<std::unique_ptr<StorageManager>, std::error_code> StorageManager::Create(
    ILogger& logger) {
  logger.LogInfo("Creating StorageManager");

  auto storage_manager = std::unique_ptr<StorageManager>(new StorageManager(logger));

  if (auto res = storage_manager->Init(); !res) {
    return std::unexpected(std::make_error_code(std::errc::invalid_argument));
  }

  return storage_manager;
}

StorageManager::~StorageManager() {
  logger_.LogInfo("Cleaning up Storage Mangager");
}

std::expected<void, std::error_code> StorageManager::Init() const {
  if (!std::filesystem::exists(user_files_path_)) {
    if (!std::filesystem::create_directory(user_files_path_)) {
      logger_.LogError("Error creating directory");
      return std::unexpected(std::make_error_code(std::errc::io_error));
    }
  }

  if (!std::filesystem::exists(asset_path_)) {
    if (!std::filesystem::create_directory(asset_path_)) {
      logger_.LogError("Error creating directory");
      return std::unexpected(std::make_error_code(std::errc::io_error));
    }
  }

  return {};
}

std::optional<std::vector<std::byte>> StorageManager::ReadFile(
    const std::filesystem::path& path) const {
  if (!std::filesystem::exists(path)) {
    logger_.LogError("File path does not exist: " + path.string());
    return std::nullopt;
  }
  if (!std::filesystem::is_regular_file(path)) {
    logger_.LogError("Path is not a regular file: " + path.string());
    return std::nullopt;
  }

  std::ifstream file(path, std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    logger_.LogError("Failed to open file: " + path.string());
    return std::nullopt;
  }

  const std::streamsize kSize = file.tellg();
  if (kSize < 0) {
    logger_.LogError("Error reading file: " + path.string());
    return std::nullopt;
  }

  (void) file.seekg(0, std::ios::beg);

  std::vector<std::byte> buffer(static_cast<size_t>(kSize));
  if (!file.read(reinterpret_cast<char*>(buffer.data()), kSize)) {
    logger_.LogError("Error reading file: " + path.string());
    return std::nullopt;
  }
  return buffer;
}

bool StorageManager::WriteFile(const std::filesystem::path& path,
                               std::span<const std::byte> data) const {
  std::ofstream file(path, std::ios::binary | std::ios::trunc);
  if (!file.is_open()) {
    logger_.LogError("Error opening file: " + path.string());
    return false;
  }

  if (!file.write(reinterpret_cast<const char*>(data.data()),
                  static_cast<std::streamsize>(data.size()))) {
    logger_.LogError("Error writing file: " + path.string());
    file.close();
    if (!std::filesystem::remove(path)) {
      logger_.LogError("Error removing file: " + path.string());
    }
    return false;
  }
  logger_.LogInfo("File saved successfully: " + path.string());
  return true;
}

std::optional<std::vector<std::byte>> StorageManager::LoadResource(std::string_view name) const {
  return ReadFile(GetResourcePath(name));
}

std::optional<std::vector<std::byte>> StorageManager::LoadFile(std::string_view name) const {
  return ReadFile(GetUserFilePath(name));
}

bool StorageManager::SaveFile(std::string_view name, std::span<std::byte> data) const {
  return WriteFile(GetUserFilePath(name), data);
}

bool StorageManager::SaveFile(std::string_view name, const std::vector<std::byte>& data) const {
  return WriteFile(GetUserFilePath(name), data);
}
std::vector<std::string> StorageManager::ListFiles() const {
  std::vector<std::string> files;
  std::error_code error;
  for (const auto& entry : std::filesystem::directory_iterator(user_files_path_, error)) {
    if (entry.is_regular_file()) {
      files.push_back(entry.path().filename().string());
    }
  }
  if (error) {
    logger_.LogFmt(LogLevel::ERROR, "ListFiles error: {}", error.message());
  }
  return files;
}

bool StorageManager::DeleteFile(std::string_view path) const {
  const auto kPath = GetUserFilePath(path);
  if (!std::filesystem::exists(kPath)) {
    logger_.LogError("Could not find file for deletion: " + kPath.string());
    return true;
  }

  if (!std::filesystem::remove(kPath)) {
    logger_.LogError("Error removing file: " + kPath.string());
    return false;
  }
  return true;
}
std::filesystem::path StorageManager::GetUserFilePath(std::string_view name) const {
  return user_files_path_ / name;
}
std::filesystem::path StorageManager::GetResourcePath(std::string_view name) const {
  return asset_path_ / name;
}
}  // namespace screen_controller
