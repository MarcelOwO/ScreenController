//
// Created by marce on 4/23/2025.
//

#include "storage_manager_impl.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <unordered_set>

namespace screen_controller {

namespace {

std::filesystem::path EnvironmentPath(const char* name) {
  const char* value = std::getenv(name);
  return value == nullptr ? std::filesystem::path{} : std::filesystem::path{value};
}

std::filesystem::path FindAssetPath() {
  if (auto path = EnvironmentPath("SCREEN_CONTROLLER_ASSET_DIR"); !path.empty()) {
    return path;
  }
  for (const auto& candidate :
       {std::filesystem::path{"assets"}, std::filesystem::path{"res"},
        std::filesystem::path{"/usr/share/screencontroller/assets"},
        std::filesystem::path{"/usr/local/share/screencontroller/assets"}}) {
    if (std::filesystem::exists(candidate / "startup_files")) {
      return candidate;
    }
  }
  return "assets";
}

}  // namespace

StorageManager::StorageManager(ILogger& logger)
    : logger_(logger),
      asset_path_(FindAssetPath()),
      user_files_path_(EnvironmentPath("SCREEN_CONTROLLER_STATE_DIR")) {
  if (user_files_path_.empty()) {
    user_files_path_ = "files";
  }
}

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

  if (!std::filesystem::is_directory(asset_path_)) {
    logger_.LogError("Asset directory does not exist: " + asset_path_.string());
    return std::unexpected(std::make_error_code(std::errc::no_such_file_or_directory));
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
  constexpr std::size_t kMaxFileSize = 128U * 1024U * 1024U;
  if (data.empty() || data.size() > kMaxFileSize) {
    logger_.LogError("Refusing an empty or oversized file");
    return false;
  }

  auto temporary_path = path;
  temporary_path += ".uploading";
  std::ofstream file(temporary_path, std::ios::binary | std::ios::trunc);
  if (!file.is_open()) {
    logger_.LogError("Error opening temporary file: " + temporary_path.string());
    return false;
  }

  if (!file.write(reinterpret_cast<const char*>(data.data()),
                  static_cast<std::streamsize>(data.size()))) {
    logger_.LogError("Error writing file: " + path.string());
    file.close();
    std::error_code ignored;
    std::filesystem::remove(temporary_path, ignored);
    return false;
  }
  file.close();

  std::error_code error;
  std::filesystem::rename(temporary_path, path, error);
  if (error) {
    logger_.LogFmt(LogLevel::ERROR, "Failed to publish uploaded file: {}", error.message());
    std::filesystem::remove(temporary_path, error);
    return false;
  }
  logger_.LogInfo("File saved successfully: " + path.string());
  return true;
}

std::optional<std::vector<std::byte>> StorageManager::LoadResource(std::string_view name) const {
  return ReadFile(GetResourcePath(name));
}

std::optional<std::vector<std::byte>> StorageManager::LoadFile(std::string_view name) const {
  if (!IsValidFilename(name)) {
    return std::nullopt;
  }
  return ReadFile(GetUserFilePath(name));
}

bool StorageManager::SaveFile(std::string_view name, std::span<const std::byte> data) const {
  if (!IsValidFilename(name)) {
    return false;
  }
  return WriteFile(GetUserFilePath(name), data);
}

bool StorageManager::SaveFile(std::string_view name, const std::vector<std::byte>& data) const {
  if (!IsValidFilename(name)) {
    return false;
  }
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
  std::ranges::sort(files);
  return files;
}

bool StorageManager::DeleteFile(std::string_view path) const {
  if (!IsValidFilename(path)) {
    return false;
  }
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
  if (!IsValidFilename(name)) {
    return {};
  }
  return user_files_path_ / name;
}
std::filesystem::path StorageManager::GetResourcePath(std::string_view name) const {
  return asset_path_ / name;
}

bool StorageManager::IsValidFilename(const std::string_view name) const {
  if (name.empty() || name.size() > 255 || name == "." || name == "..") {
    logger_.LogWarn("Rejected invalid filename");
    return false;
  }
  const std::filesystem::path path{name};
  if (path.has_parent_path() || path.filename() != path) {
    logger_.LogWarn("Rejected filename containing a path");
    return false;
  }

  std::string extension = path.extension().string();
  std::ranges::transform(extension, extension.begin(),
                         [](const unsigned char value) { return std::tolower(value); });
  static const std::unordered_set<std::string> kSupportedExtensions = {
      ".jpg", ".jpeg", ".png", ".bmp", ".gif", ".mp4", ".webm", ".webp"};
  if (!kSupportedExtensions.contains(extension)) {
    logger_.LogWarn("Rejected unsupported file extension");
    return false;
  }
  return true;
}
}  // namespace screen_controller
