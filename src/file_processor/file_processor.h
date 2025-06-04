//
// Created by marce on 4/23/2025.
//

#ifndef FILE_PROCESSOR_H
#define FILE_PROCESSOR_H

#include <optional>

#include "../common/file_type.h"
#include "decoders/decoder.h"
namespace screen_controller {
class FileProcessor {
 public:
  FileProcessor();
  ~FileProcessor();

  FileProcessor(const FileProcessor&) = delete;
  FileProcessor& operator=(const FileProcessor&) = delete;
  FileProcessor(FileProcessor&&) = delete;
  FileProcessor& operator=(FileProcessor&&) = delete;

  bool init();
  bool process_file(std::string_view path);

  std::optional<std::unique_ptr<processing::models::FrameData>> get_processed_data() const;

 private:
  std::unique_ptr<processing::Decoder> decoder_;
  static common::FileType get_type(std::string_view name);
};
}  // namespace screen_controller
#endif  // FILE_PROCESSOR_H
