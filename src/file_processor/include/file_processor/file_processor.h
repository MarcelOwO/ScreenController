//
// Created by marce on 4/23/2025.
//

#ifndef FILE_PROCESSOR_H
#define FILE_PROCESSOR_H
#include "../../decoders/decoder.h"
#include <file_type.h>
#include <logging/logger.h>

#include <expected>
#include <memory>
#include <optional>

#include "models/error_enum.h"
#include "models/frame_data.h"

namespace screen_controller {
class FileProcessor {
public:
  explicit FileProcessor(const std::shared_ptr<Logger>& logger);
  ~FileProcessor();

  FileProcessor(const FileProcessor&) = delete;
  FileProcessor& operator=(const FileProcessor&) = delete;
  FileProcessor(FileProcessor&&) = delete;
  FileProcessor& operator=(FileProcessor&&) = delete;

  bool process_file(std::string_view path);

  [[nodiscard]] std::optional<std::unique_ptr<common::FrameData>>
  get_processed_data() const;

private:
  std::shared_ptr<Logger> logger_;
  std::unique_ptr<processing::Decoder> decoder_;
  static common::FileType get_type(std::string_view name);
};
} // namespace screen_controller
#endif  // FILE_PROCESSOR_H