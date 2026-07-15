#include "video_decoder.hpp"

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

namespace screen_controller::processing {

VideoDecoder::VideoDecoder(const std::string_view path, ILogger& logger)
    : logger_(logger), path_(path) {
  logger_.LogFmt(LogLevel::INFO, "Creating VideoDecoder for path: {}", path);
}

bool VideoDecoder::Init() {
  if (avformat_open_input(&format_context_, path_.c_str(), nullptr, nullptr) < 0 ||
      avformat_find_stream_info(format_context_, nullptr) < 0) {
    logger_.LogFmt(LogLevel::ERROR, "Failed to open video: {}", path_);
    return false;
  }

  video_stream_index_ =
      av_find_best_stream(format_context_, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
  if (video_stream_index_ < 0) {
    logger_.LogError("Could not find a video stream");
    return false;
  }

  const AVStream* stream = format_context_->streams[video_stream_index_];
  const AVCodec* codec = avcodec_find_decoder(stream->codecpar->codec_id);
  if (codec == nullptr) {
    logger_.LogError("Could not find the video decoder");
    return false;
  }

  codec_context_ = avcodec_alloc_context3(codec);
  if (codec_context_ == nullptr ||
      avcodec_parameters_to_context(codec_context_, stream->codecpar) < 0 ||
      avcodec_open2(codec_context_, codec, nullptr) < 0) {
    logger_.LogError("Could not initialize the video decoder");
    return false;
  }

  constexpr int kMaxDimension = 4096;
  if (codec_context_->width <= 0 || codec_context_->height <= 0 ||
      codec_context_->width > kMaxDimension || codec_context_->height > kMaxDimension) {
    logger_.LogError("Video dimensions are invalid or exceed 4096 pixels");
    return false;
  }

  frame_ = av_frame_alloc();
  packet_ = av_packet_alloc();
  if (frame_ == nullptr || packet_ == nullptr) {
    logger_.LogError("Could not allocate FFmpeg frame state");
    return false;
  }

  frame_data_ = {
      .data = std::vector<uint8_t>(static_cast<std::size_t>(codec_context_->width) *
                                       static_cast<std::size_t>(codec_context_->height) * 3U,
                                   0U),
      .width = codec_context_->width,
      .height = codec_context_->height,
      .channels = 3,
  };
  sws_ctx_ = sws_getContext(codec_context_->width, codec_context_->height, codec_context_->pix_fmt,
                            codec_context_->width, codec_context_->height, AV_PIX_FMT_RGB24,
                            SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
  if (sws_ctx_ == nullptr) {
    logger_.LogError("Could not initialize video pixel conversion");
    return false;
  }

  double frame_rate = av_q2d(stream->avg_frame_rate);
  if (!std::isfinite(frame_rate) || frame_rate < 1.0 || frame_rate > 120.0) {
    frame_rate = 30.0;
  }
  frame_interval_ = std::chrono::duration_cast<std::chrono::steady_clock::duration>(
      std::chrono::duration<double>(1.0 / frame_rate));
  next_frame_due_ = std::chrono::steady_clock::now();
  initialized_ = true;
  return true;
}

bool VideoDecoder::ScaleFrame() {
  auto* destination = frame_data_.data.data();
  const int destination_stride = frame_data_.width * frame_data_.channels;
  const int rows = sws_scale(sws_ctx_, frame_->data, frame_->linesize, 0, codec_context_->height,
                             &destination, &destination_stride);
  av_frame_unref(frame_);
  return rows == frame_data_.height;
}

bool VideoDecoder::Rewind() {
  if (av_seek_frame(format_context_, video_stream_index_, 0, AVSEEK_FLAG_BACKWARD) < 0) {
    logger_.LogError("Could not rewind video");
    return false;
  }
  avcodec_flush_buffers(codec_context_);
  input_eof_ = false;
  return true;
}

bool VideoDecoder::DecodeNextFrame() {
  for (;;) {
    const int receive_result = avcodec_receive_frame(codec_context_, frame_);
    if (receive_result == 0) {
      return ScaleFrame();
    }
    if (receive_result == AVERROR_EOF) {
      if (!Rewind()) {
        return false;
      }
      continue;
    }
    if (receive_result != AVERROR(EAGAIN)) {
      logger_.LogFmt(LogLevel::ERROR, "Video decode failed: {}", receive_result);
      return false;
    }

    bool submitted_packet = false;
    while (!submitted_packet) {
      const int read_result = av_read_frame(format_context_, packet_);
      if (read_result < 0) {
        if (!input_eof_) {
          input_eof_ = true;
          (void) avcodec_send_packet(codec_context_, nullptr);
          submitted_packet = true;
          continue;
        }
        if (!Rewind()) {
          return false;
        }
        continue;
      }
      if (packet_->stream_index == video_stream_index_) {
        const int send_result = avcodec_send_packet(codec_context_, packet_);
        av_packet_unref(packet_);
        if (send_result < 0 && send_result != AVERROR(EAGAIN)) {
          logger_.LogFmt(LogLevel::ERROR, "Submitting a video packet failed: {}", send_result);
          return false;
        }
        submitted_packet = true;
      } else {
        av_packet_unref(packet_);
      }
    }
  }
}

bool VideoDecoder::HasData() {
  return initialized_ && std::chrono::steady_clock::now() >= next_frame_due_;
}

std::optional<std::unique_ptr<FrameData>> VideoDecoder::GetNextFrame() {
  if (!HasData() || !DecodeNextFrame()) {
    return std::nullopt;
  }
  next_frame_due_ = std::chrono::steady_clock::now() + frame_interval_;
  return std::make_unique<FrameData>(frame_data_);
}

VideoDecoder::~VideoDecoder() {
  sws_freeContext(sws_ctx_);
  av_frame_free(&frame_);
  av_packet_free(&packet_);
  avcodec_free_context(&codec_context_);
  avformat_close_input(&format_context_);
}

}  // namespace screen_controller::processing
