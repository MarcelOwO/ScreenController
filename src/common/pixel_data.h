//
// Created by marce on 4/23/2025.
//

#ifndef FRAME_DATA_H
#define FRAME_DATA_H

#include <cstdint>



struct PixelData {

uint8_t r;
uint8_t g;
uint8_t b;

  PixelData(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) :
      r(red), g(green), b(blue) {}

 PixelData() : r(0), g(0), b(0) {}


};



#endif //FRAME_DATA_H
