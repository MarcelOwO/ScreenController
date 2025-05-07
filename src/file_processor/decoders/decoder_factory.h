//
// Created by marce on 5/7/2025.
//

#ifndef DECODER_FACTORY_H
#define DECODER_FACTORY_H
#include <memory>
#include <string_view>

#include "decoder.h"

namespace screen_controller::common {
enum class FileType;
}

namespace screen_controller::processing {

class DecoderFactory {
public:
  static std::unique_ptr<processing::Decoder> create(std::string_view name,
                                                    common::FileType type);

};

} // screen_controller

#endif //DECODER_FACTORY_H
