//
// Created by marce on 5/6/2025.
//
#pragma once

#include <string>

#include <logging/logger.hpp>
#include "decoder.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

namespace screen_controller::processing {
class VideoDecoder final : public IDecoder {
public:
  explicit VideoDecoder(std::string_view path, ILogger& logger);
  bool Init() override;
  ~VideoDecoder() override;
  bool HasData() override;
  std::optional<std::unique_ptr<FrameData>> GetNextFrame() override;

private:
  bool ScaleFrame();

  bool looped_;
  int frame_count_;
  int current_frame_;
  double frame_rate_;
  int video_stream_index_;
  ILogger& logger_;

  std::vector<FrameData> frames_;
  FrameData frame_data_;

  std::string path_;

  AVCodecContext* codec_context_;
  AVFormatContext* format_context_;

  AVFrame* frame_;
  AVPacket* packet_;

  SwsContext* sws_ctx_;
};
}  // namespace screen_controller::processing
