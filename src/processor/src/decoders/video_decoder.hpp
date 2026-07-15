#pragma once

#include <chrono>
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
  ~VideoDecoder() override;

  VideoDecoder(const VideoDecoder&) = delete;
  VideoDecoder& operator=(const VideoDecoder&) = delete;
  VideoDecoder(VideoDecoder&&) = delete;
  VideoDecoder& operator=(VideoDecoder&&) = delete;

  bool Init() override;
  bool HasData() override;
  std::optional<std::unique_ptr<FrameData>> GetNextFrame() override;

private:
  bool DecodeNextFrame();
  bool ScaleFrame();
  bool Rewind();

  bool initialized_{false};
  bool input_eof_{false};
  int video_stream_index_{-1};
  ILogger& logger_;
  FrameData frame_data_;
  std::string path_;
  std::chrono::steady_clock::duration frame_interval_{std::chrono::milliseconds{33}};
  std::chrono::steady_clock::time_point next_frame_due_{};

  AVCodecContext* codec_context_{nullptr};
  AVFormatContext* format_context_{nullptr};
  AVFrame* frame_{nullptr};
  AVPacket* packet_{nullptr};
  SwsContext* sws_ctx_{nullptr};
};

}  // namespace screen_controller::processing
