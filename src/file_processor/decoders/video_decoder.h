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
  virtual bool init() override;
  virtual ~VideoDecoder() override;

  std::optional<std::shared_ptr<models::FrameData>> get_next_frame() override;

 private:
  std::vector<models::FrameData> video_data_;
  bool looped_;
  int frame_count_;
  int current_frame_;

  models::FrameData image_data_;
  std::string_view path_;

  AVCodecContext* codec_context_;
  AVFormatContext* format_context_;
  int video_stream_index_;

  AVFrame* frame_;
  AVPacket* packet_;

  SwsContext* sws_ctx_;
};
}  // namespace screen_controller::processing

#endif  // VIDEO_PROCESSOR_H
