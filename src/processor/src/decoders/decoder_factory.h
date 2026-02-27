//
// Created by marce on 5/7/2025.
//

#ifndef DECODER_FACTORY_H
#define DECODER_FACTORY_H
#include <memory>
#include <string_view>

#include "decoder.h"
#include "file_type.h"
#include "logging/logger.h"

namespace screen_controller::processing {

class DecoderFactory {
 public:
  static std::unique_ptr<IDecoder> create(std::string_view name,
                                          common::FileType type,
                                          ILogger& logger);
};

}  // namespace screen_controller::processing

#endif  // DECODER_FACTORY_H
