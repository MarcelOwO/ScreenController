//
// Created by marce on 5/6/2025.
//

#ifndef IMAGE_DATA_H
#define IMAGE_DATA_H
#include <cstdint>
#include <vector>

namespace screen_controller::processing::models {

struct FrameData {
  std::vector<uint8_t> data;
  int width;
  int height;
  int channels;
};

}  // namespace screen_controller::processing::models

#endif  // IMAGE_DATA_H
