//
// Created by marce on 4/23/2025.
//
#include "file_processor_impl.hpp"

#include <enums/file_type.hpp>
#include <processor/file_processor.hpp>

#include <filesystem>
#include <unordered_map>

#include "decoders/decoder_factory.hpp"

namespace screen_controller {

std::expected<std::unique_ptr<FileProcessor>, std::error_code> FileProcessor::Create(
    ILogger& logger) {
  auto file_processor = std::unique_ptr<FileProcessor>(new FileProcessor(logger));

  if (!file_processor) {
    return std::unexpected(std::make_error_code(std::errc::invalid_argument));
  }

  return file_processor;
}

FileProcessor::FileProcessor(ILogger& logger) : logger_(logger) {
  logger.LogInfo("Creating FileProcessor");
};

FileProcessor::~FileProcessor() {
  logger_.LogInfo("Cleaning up File Processor");
  if (decoder_ != nullptr) {
    decoder_.reset();
  }
};

bool FileProcessor::ProcessFile(const std::string_view path) {
  const auto type = get_type(path);

  if (type == FileType::kNone) {
    logger_.LogError("File type not supported: " + std::string(path));
    return false;
  }

  decoder_ = processing::DecoderFactory::Create(path, type, logger_);

  if (decoder_ == nullptr) {
    logger_.LogError("Decoder not supported for file: " + std::string(path));
    return false;
  }

  if (!decoder_->Init()) {
    logger_.LogError("Failed to initialize decoder for file: " + std::string(path));
    return false;
  }

  return true;
}

std::optional<std::unique_ptr<FrameData>> FileProcessor::GetProcessedData() const {
  if (!decoder_) {
    logger_.LogError("Decoder not initialized");
    return std::nullopt;
  }

  if (!decoder_->HasData()) {
    return std::nullopt;
  }

  return decoder_->GetNextFrame();
}

FileType FileProcessor::get_type(const std::string_view name) {
  const auto ext = std::filesystem::path(name).extension();
  if (ext.empty() || ext == ".") {
    return FileType::kNone;
  }

  static const std::unordered_map<std::string, FileType> ext_map = {
      {".jpg", FileType::kJpg},   {".jpeg", FileType::kJpg}, {".png", FileType::kPng},
      {".gif", FileType::kGif},   {".bmp", FileType::kBmp},  {".mp4", FileType::kMp4},
      {".webp", FileType::kWebp}, {".webm", FileType::kWebm}};

  if (!ext_map.contains(ext)) {
    return FileType::kNone;
  }

  return ext_map.at(ext);
}
}  // namespace screen_controller
