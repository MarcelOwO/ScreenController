//
// Created by marce on 4/23/2025.
//

#ifndef FRAME_DATA_H
#define FRAME_DATA_H

#include <cstdint>

namespace screen_controller::common {

struct PixelData {
 public:
  PixelData(const uint8_t red, const uint8_t green, const uint8_t blue)
      : r(red), g(green), b(blue) {}

  PixelData() : r(), g(), b() {}

 private:
  uint8_t r;
  uint8_t g;
  uint8_t b;
};
}  // namespace screen_controller::common

#endif  // FRAME_DATA_H
