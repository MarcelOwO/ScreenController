//
// Created by marce on 5/6/2025.
//

#include "video_decoder.hpp"

#include <fstream>
#include <iostream>
#include <vector>

namespace screen_controller::processing {
VideoDecoder::VideoDecoder(const std::string_view path, ILogger& logger)
    : video_stream_index_(-1),
      logger_(logger),
      frame_data_{.data = std::vector<uint8_t>(static_cast<size_t>(1920 * 1080 * 3),
                                               static_cast<uint8_t>(0)),
                  .width = 1920,
                  .height = 1080,
                  .channels = 3},
      path_(path),
      codec_context_(nullptr),
      format_context_(nullptr),
      frame_(nullptr),
      packet_(nullptr),
      sws_ctx_(nullptr) {
  logger_.LogInfo("Creating VideoDecoder for path: " + std::string(path));
}

bool VideoDecoder::init() {
  if (avformat_open_input(&format_context_, path_.data(), nullptr, nullptr) < 0) {
    logger_.LogError("Failed to open video file: " + std::string(path_));
    return false;
  }

  if (avformat_find_stream_info(format_context_, nullptr) < 0) {
    logger_.LogError("Failed to find stream info for file: " + std::string(path_));
    return false;
  }

  for (unsigned int i = 0U; i < format_context_->nb_streams; ++i) {
    if (format_context_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      video_stream_index_ = i;
      break;
    }
  }

  if (video_stream_index_ == -1) {
    logger_.LogError("Could not find a video stream");
    return false;
  }

  const auto codec_id = format_context_->streams[video_stream_index_]->codecpar->codec_id;
  const AVCodec* codec = avcodec_find_decoder(codec_id);

  if (codec == nullptr) {
    logger_.LogError("Failed to find decoder for codec ID: {} " + std::to_string(codec_id));
    return false;
  }

  codec_context_ = avcodec_alloc_context3(codec);

  if (codec_context_ == nullptr) {
    logger_.LogError("Could not allocate codec context");
    return false;
  }

  if (avcodec_parameters_to_context(codec_context_,
                                    format_context_->streams[video_stream_index_]->codecpar) < 0) {
    logger_.LogError("Could not copy codec parameters to context");
    return false;
  }

  const auto [num, den] = format_context_->streams[video_stream_index_]->avg_frame_rate;

  frame_rate_ = av_q2d(format_context_->streams[video_stream_index_]->avg_frame_rate);

  const char* drm_device = "/dev/dri/renderD128";  // Try explicit render node

  if (av_hwdevice_ctx_create(&codec_context_->hw_device_ctx, AV_HWDEVICE_TYPE_DRM,
                             drm_device,  // Pass the device path here
                             nullptr, 0) < 0) {
    logger_.LogError("Could not create DRM hardware device context at " + std::string(drm_device));
    return false;
  }
  if (avcodec_open2(codec_context_, codec, nullptr) < 0) {
    logger_.LogError("Could not open codec");
    return false;
  }

  frame_ = av_frame_alloc();
  packet_ = av_packet_alloc();

  if (frame_ == nullptr || packet_ == nullptr) {
    logger_.LogError("Could not allocate AVPacket");
    return false;
  }

  sws_ctx_ =
      sws_getContext(codec_context_->width, codec_context_->height, codec_context_->pix_fmt, 1920,
                     1080, AV_PIX_FMT_RGB24, SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
  if (sws_ctx_ == nullptr) {
    logger_.LogError("Could not create sws context");
    return false;
  }

  while (av_read_frame(format_context_, packet_) >= 0) {
    if (packet_->stream_index != video_stream_index_) {
      av_packet_unref(packet_);
      logger_.LogError("Could not read frame");
      return false;
    }

    if (const auto ret = avcodec_send_packet(codec_context_, packet_); ret < 0) {
      logger_.LogError("avcodec_send_packet error");
      return false;
    }

    if (const auto ret = avcodec_receive_frame(codec_context_, frame_); ret != 0) {
      logger_.LogError("avcodec_receive_frame error");
      return false;
    }

    av_packet_unref(packet_);

    if (frame_->data == nullptr) {
      logger_.LogError("Frame data is null");
      return false;
    }

    auto* dst_data = frame_data_.data.data();
    constexpr int dst_linesize = 1920 * 3;

    if (sws_scale(sws_ctx_, frame_->data, frame_->linesize, 0, codec_context_->height, &dst_data,
                  &dst_linesize) < 0) {
      logger_.LogError("Could not scale frame");
      std::cerr << "Error: Failed to scale frame" << std::endl;
      return false;
    }

    frames_.push_back(frame_data_);
    frame_count_++;
  }

  (void) avcodec_send_packet(codec_context_, nullptr);
  while (avcodec_receive_frame(codec_context_, frame_) == 0) {
  }

  looped_ = true;
  current_frame_ = 0;

  (void) av_seek_frame(format_context_, video_stream_index_, 0, AVSEEK_FLAG_BACKWARD);

  avcodec_flush_buffers(codec_context_);

  return true;
}

VideoDecoder::~VideoDecoder() {
  if (sws_ctx_ != nullptr) {
    sws_freeContext(sws_ctx_);
    sws_ctx_ = nullptr;
  }
  if (frame_ != nullptr) {
    av_frame_free(&frame_);
    frame_ = nullptr;
  }
  if (packet_ != nullptr) {
    av_packet_free(&packet_);
    packet_ = nullptr;
  }
  if (codec_context_ != nullptr) {
    avcodec_free_context(&codec_context_);
    codec_context_ = nullptr;
  }
  if (format_context_ != nullptr) {
    avformat_close_input(&format_context_);
    format_context_ = nullptr;
  }
}
bool VideoDecoder::has_data() {
  return looped_;
}

std::optional<std::unique_ptr<FrameData>> VideoDecoder::get_next_frame() {
  const FrameData& frame = frames_.at(static_cast<size_t>(current_frame_));

  current_frame_++;

  if (current_frame_ >= frame_count_) {
    current_frame_ = 0;
  }

  return std::make_unique<FrameData>(frame);
}

}  // namespace screen_controller::processing
