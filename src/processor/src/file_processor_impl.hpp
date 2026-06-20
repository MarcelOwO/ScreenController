//
// Created by marce on 4/23/2025.
//
#pragma once

#include <enums/file_type.hpp>
#include <logging/logger.hpp>
#include <processor/file_processor.hpp>

#include <expected>
#include <memory>
#include <optional>

#include "decoders/decoder.hpp"
#include "models/frame_data.hpp"

namespace screen_controller {

class FileProcessor : public IFileProcessor {
public:
  ~FileProcessor();

  static std::expected<std::unique_ptr<FileProcessor>, std::error_code> Create(ILogger& logger);

  FileProcessor(const FileProcessor&) = delete;
  FileProcessor& operator=(const FileProcessor&) = delete;
  FileProcessor(FileProcessor&&) = delete;
  FileProcessor& operator=(FileProcessor&&) = delete;

  bool ProcessFile(std::string_view path);

  [[nodiscard]] std::optional<std::unique_ptr<FrameData>> GetProcessedData() const;

private:
  FileProcessor(ILogger& logger);

  ILogger& logger_;
  std::unique_ptr<processing::IDecoder> decoder_;
  static FileType get_type(std::string_view name);
};
}  // namespace screen_controller
