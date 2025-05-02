//
// Created by marce on 4/23/2025.
//

#include "file_processor.h"

#include <filesystem>
#include <string>

#include "../common/file_type.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../../external/stb/stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <fstream>
#include <iostream>
#include <unordered_map>

#include "../../external/stb/stb_image_resize2.h"

namespace screen_controller {
FileProcessor::FileProcessor() : data_ready_(false), processed_data_() {}

FileProcessor::~FileProcessor() = default;

bool FileProcessor::init() {
  processed_data_.reserve(static_cast<size_t>(1920 * 1080 * 3));
  return true;
}

void FileProcessor::clear_data() {
  processed_data_.clear();
  data_ready_ = false;
}

std::optional<std::span<const std::byte>> FileProcessor::get_processed_data()
    const {
  if (!data_ready_) {
    std::cerr << " data not ready" << std::endl;
    return std::nullopt;
  }

  if (processed_data_.empty()) {
    std::cerr << " no processed data" << std::endl;
    return std::nullopt;
  }

  return processed_data_;
}

bool FileProcessor::process_file(std::string_view path) {
  auto type = get_type(path);

  if (type == common::FileType::kNone) {
    std::cout << "FileProcessor::process_file: unknown file type" << std::endl;
    return false;
  }

  int x;
  int y;
  int n;

  switch (type) {
    case common::FileType::kJpg: {
      [[fallthrough]];
    }
    case common::FileType::kPng: {
      [[fallthrough]];
    }
    case common::FileType::kBmp: {
      const auto data = stbi_load(path.data(), &x, &y, &n, 3);

      if (!data) {
        std::cerr << "FileProcessor::process_file: failed to load image"
                  << std::endl;
        return false;
      }

      resize_file(data, x, y);

      stbi_image_free(data);

      return true;
    }
    case common::FileType::kWebp: {
      std::ifstream file(path.data(), std::ios::binary | std::ios::ate);

      if (!file) {
        std::cerr << "Failed to open file" << "\n";
        return false;
      }

      std::streamsize size = file.tellg();

      if (size == -1) {
        std::cerr << "Failed to read file: " << path << std::endl;
        return false;
      }

      file.seekg(0, std::ios::beg);

      std::vector<uint8_t> buffer(size);

      if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        std::cerr << "Failed to read file: " << path << std::endl;
        return false;
      }

      uint8_t* decoded_data =
          WebPDecodeRGB(buffer.data(), buffer.size(), &x, &y);

      if (!decoded_data) {
        std::cerr << "Failed to decode file: " << path << std::endl;
        return false;
      }
      resize_file(decoded_data, x, y);

      WebPFree(decoded_data);

      return true;
    }
    default: {
      return false;
    }
  }
}

void FileProcessor::resize_file(const uint8_t* data, const int x, const int y) {
  if (x == 1920 && y == 1080) {
    std::cout << "no need to resize" << std::endl;
    return;
  }

  const auto out =
      stbir_resize_uint8_srgb(data, x, y, 0, nullptr, 1920, 1080, 0, STBIR_RGB);

  if (!out) {
    std::cerr << "Failed to resize image" << std::endl;
    return;
  }

  const auto a = reinterpret_cast<std::byte*>(out);

  processed_data_.assign(a, a + 1920 * 1080 * 3);

  data_ready_ = true;
}

common::FileType FileProcessor::get_type(std::string_view name) {
  const auto ext = std::filesystem::path(name).extension();
  if (ext == ".") {
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
