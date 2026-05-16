//
// Created by marce on 4/23/2025.
//

#ifndef FILE_PROCESSOR_H
#define FILE_PROCESSOR_H

#include <file_type.h>
#include <logging/logger.h>
#include <processor/file_processor.h>

#include <expected>
#include <memory>
#include <optional>

#include "decoders/decoder.h"
#include "models/frame_data.h"

namespace screen_controller {

class FileProcessor : public IFileProcessor {
 public:
  ~FileProcessor();

  static std::expected<std::unique_ptr<FileProcessor>, std::error_code> create(
      ILogger& logger);

  FileProcessor(const FileProcessor&) = delete;
  FileProcessor& operator=(const FileProcessor&) = delete;
  FileProcessor(FileProcessor&&) = delete;
  FileProcessor& operator=(FileProcessor&&) = delete;

  bool process_file(std::string_view path);

  [[nodiscard]] std::optional<std::unique_ptr<common::FrameData>>
  get_processed_data() const;

 private:
  FileProcessor(ILogger& logger);

  ILogger& logger_;
  std::unique_ptr<processing::IDecoder> decoder_;
  static common::FileType get_type(std::string_view name);
};
}  // namespace screen_controller
#endif  // FILE_PROCESSOR_H
