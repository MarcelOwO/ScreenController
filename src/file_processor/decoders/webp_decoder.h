//
// Created by marce on 5/6/2025.
//

#ifndef WEBP_PROCESSOR_H
#define WEBP_PROCESSOR_H
#include <optional>
#include <string_view>

#include "decoder.h"
#include "models/frame_data.h"

namespace screen_controller::processing {

class WebpDecoder final : public Decoder {
 public:
  explicit WebpDecoder(std::string_view path);

  virtual ~WebpDecoder() override = default;

  WebpDecoder(const WebpDecoder&) = delete;
  WebpDecoder& operator=(const WebpDecoder&) = delete;
  WebpDecoder(WebpDecoder&&) = delete;
  WebpDecoder& operator=(WebpDecoder&&) = delete;
  virtual std::optional<std::shared_ptr<models::FrameData>> get_next_frame() override;
  //static std::optional<models::FrameData> WebpDecoder::process_file(
   //   std::string_view path);

private:
  bool is_loaded_;
  std::string_view path_;
};

}  // namespace screen_controller::processing

#endif  // WEBP_PROCESSOR_H
