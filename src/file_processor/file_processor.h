//
// Created by marce on 4/23/2025.
//

#ifndef FILE_PROCESSOR_H
#define FILE_PROCESSOR_H

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <webp/decode.h>

#include <array>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "../common/file_type.h"
#include "../common/pixel_data.h"

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
  void resize_file(const uint8_t* data, int x, int y);

  std::optional<std::span<const std::byte>> get_processed_data() const;
  void clear_data();

 private:
  bool data_ready_;
  std::vector<std::byte> processed_data_;
  common::FileType get_type(std::string_view name);
};
}  // namespace screen_controller
#endif  // FILE_PROCESSOR_H
