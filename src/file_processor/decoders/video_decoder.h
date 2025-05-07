//
// Created by marce on 5/6/2025.
//

#ifndef VIDEO_PROCESSOR_H
#define VIDEO_PROCESSOR_H

#include <string_view>

#include "decoder.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

namespace screen_controller::processing {

class VideoDecoder final : public Decoder {
 public:
  explicit VideoDecoder(std::string_view path);
  bool init();
  virtual ~VideoDecoder() override = default;

  bool process_video(std::string_view path);

 private:
  std::string_view path_;
  bool decode(AVCodecContext* dec_ctx, AVFrame* frame, const AVPacket* pkt);
};
}  // namespace screen_controller::processing

#endif  // VIDEO_PROCESSOR_H
