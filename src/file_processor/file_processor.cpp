//
// Created by marce on 4/23/2025.
//

#include <file_processor.h>

#include <filesystem>
#include <iostream>
#include <unordered_map>

#include "../common/file_type.h"
#include "decoders/decoder_factory.h"
#include "decoders/stb_decoder.h"

namespace screen_controller {

FileProcessor::FileProcessor() = default;

FileProcessor::~FileProcessor() = default;

bool FileProcessor::init() { return true; }

bool FileProcessor::process_file(const std::string_view path) {
  const auto type = get_type(path);

  if (type == common::FileType::kNone) {
    std::cout << "FileProcessor::process_file: unknown file type" << std::endl;
    return false;
  }

  decoder_ = processing::DecoderFactory::create(path, type);

  if (!decoder_) {
    std::cerr << "FileProcessor::process_file: failed to create decoder"
              << std::endl;
    return false;
  }

  if (!decoder_->init()) {
    std::cerr << "FileProcessor::process_file: failed to initialize decoder"
              << std::endl;
    decoder_.reset();

    return false;
  }
  return true;
}

std::optional<std::shared_ptr<processing::models::FrameData>>
FileProcessor::get_processed_data() const {
  if (!decoder_) {
    std::cerr << "FileProcessor::get_processed_data: failed to get "
                 "decoder";

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
