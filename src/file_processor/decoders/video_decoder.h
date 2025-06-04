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
  virtual bool has_data() override;
  std::optional<std::unique_ptr<models::FrameData>> get_next_frame() override;

 private:
  bool looped_;
  int frame_count_;
  int current_frame_;
  double frame_rate_;
  int video_stream_index_;

  std::vector<models::FrameData> frames_;
  models::FrameData frame_data_;

  std::string_view path_;

  AVCodecContext* codec_context_;
  AVFormatContext* format_context_;

  AVFrame* frame_;
  AVPacket* packet_;

  SwsContext* sws_ctx_;
};
}  // namespace screen_controller::processing

#endif  // VIDEO_PROCESSOR_H
