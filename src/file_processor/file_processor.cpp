//
// Created by marce on 4/23/2025.
//
#include <file_type.h>
#include <include/file_processor/file_processor.h>

#include <filesystem>
#include <iostream>
#include <unordered_map>

#include "decoders/decoder_factory.h"
#include "decoders/stb_decoder.h"

namespace screen_controller {

FileProcessor::FileProcessor(const std::shared_ptr<Logger> &logger)
    : logger_(logger) {
  logger->LogInfo("Creating FileProcessor");
};

FileProcessor::~FileProcessor() {
  if (decoder_ != nullptr) {
    decoder_.reset();
  }
};

bool FileProcessor::process_file(const std::string_view path) {
  const auto type = get_type(path);

  if (type == common::FileType::kNone) {
    logger_->LogError("File type not supported: " + std::string(path));
    return false;
  }

  decoder_ = processing::DecoderFactory::create(path, type, logger_);

  if (decoder_ == nullptr) {
    logger_->LogError("Decoder not supported for file: " + std::string(path));
    return false;
  }

  if (!decoder_->init()) {
    logger_->LogError("Failed to initialize decoder for file: " +
                      std::string(path));
    return false;
  }

  return true;
}

std::optional<std::unique_ptr<common::FrameData>>
FileProcessor::get_processed_data() const {
  if (!decoder_) {
    logger_->LogError("Decoder not initialized");
    return std::nullopt;
  }

  if (!decoder_->has_data()) {
    return std::nullopt;
  }

  return decoder_->get_next_frame();
}

common::FileType FileProcessor::get_type(const std::string_view name) {
  const auto ext = std::filesystem::path(name).extension();
  if (ext.empty() || ext == ".") {
    return common::FileType::kNone;
  }

  static const std::unordered_map<std::string, common::FileType> ext_map = {
      {".jpg", common::FileType::kJpg},   {".jpeg", common::FileType::kJpg},
      {".png", common::FileType::kPng},   {".gif", common::FileType::kGif},
      {".bmp", common::FileType::kBmp},   {".mp4", common::FileType::kMp4},
      {".webp", common::FileType::kWebp}, {".webm", common::FileType::kWebm}};

  if (!ext_map.contains(ext)) {
    return common::FileType::kNone;
  }

  return ext_map.at(ext);
}
}  // namespace screen_controller
