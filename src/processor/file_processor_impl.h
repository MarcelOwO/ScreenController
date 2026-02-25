//
// Created by marce on 4/23/2025.
//

#ifndef FILE_PROCESSOR_H
#define FILE_PROCESSOR_H

#include <file_type.h>
#include <logging/logger.h>
#include <processor/file_processor.h>

#include <memory>
#include <optional>

#include "decoders/decoder.h"
#include "models/frame_data.h"

namespace screen_controller {

class FileProcessor : IFileProcessor {
 public:
  explicit FileProcessor(ILogger& logger);
  ~FileProcessor();

  FileProcessor(const FileProcessor&) = delete;
  FileProcessor& operator=(const FileProcessor&) = delete;
  FileProcessor(FileProcessor&&) = delete;
  FileProcessor& operator=(FileProcessor&&) = delete;

  bool process_file(std::string_view path);

  [[nodiscard]] std::optional<std::unique_ptr<common::FrameData>>
  get_processed_data() const;

 private:
  ILogger& logger_;
  std::unique_ptr<processing::Decoder> decoder_;
  static common::FileType get_type(std::string_view name);
};
}  // namespace screen_controller
#endif  // FILE_PROCESSOR_H
