//
// Created by marce on 5/6/2025.
//

#include "video_decoder.h"

#include <ng-log/logging.h>

#include <fstream>
#include <iostream>
#include <vector>

namespace screen_controller::processing {
VideoDecoder::VideoDecoder(const std::string_view path)
    : video_stream_index_(-1),
      frames_{},
      frame_data_{
          .data = std::vector<uint8_t>(static_cast<size_t>(1920 * 1080 * 3),
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
  LOG(INFO) << "Created VideoDecoder";
}

bool VideoDecoder::init() {
  PCHECK(avformat_open_input(&format_context_, path_.data(), nullptr,
                             nullptr) >= 0)
      << "Failed to open video file: " << path_;

  PCHECK(avformat_find_stream_info(format_context_, nullptr) >= 0)
      << "avformat_find_stream_info error";

  for (unsigned int i = 0U; i < format_context_->nb_streams; ++i) {
    if (format_context_->streams[i]->codecpar->codec_type ==
        AVMEDIA_TYPE_VIDEO) {
      video_stream_index_ = i;
      break;
    }
  }

  PCHECK(video_stream_index_ != -1) << "Could not find a video stream";

  const auto codec_id =
      format_context_->streams[video_stream_index_]->codecpar->codec_id;
  const AVCodec* codec = avcodec_find_decoder(codec_id);

  if (codec == nullptr) {
    PLOG(ERROR) << "Failed to find decoder for codec ID: " << codec_id;
    return false;
  }

  codec_context_ = avcodec_alloc_context3(codec);

  if (codec_context_ == nullptr) {
    PLOG(ERROR) << "Could not allocate codec context";
    return false;
  }

  if (avcodec_parameters_to_context(
          codec_context_,
          format_context_->streams[video_stream_index_]->codecpar) < 0) {
    PLOG(ERROR) << "Could not copy codec parameters to context";
    return false;
  }

  const auto [num, den] =
      format_context_->streams[video_stream_index_]->avg_frame_rate;
  frame_rate_ = static_cast<double>(num / den);

  if (av_hwdevice_ctx_create(&codec_context_->hw_device_ctx,
                             AV_HWDEVICE_TYPE_DRM, nullptr, nullptr, 0) < 0) {
    PLOG(ERROR) << "Could not create hardware device context";
    return false;
  }

  if (avcodec_open2(codec_context_, codec, nullptr) < 0) {
    PLOG(ERROR) << "Could not open codec";
    return false;
  }

  frame_ = av_frame_alloc();
  packet_ = av_packet_alloc();

  if (frame_ == nullptr || packet_ == nullptr) {
    PLOG(ERROR) << "Could not allocate AVPacket";
    return false;
  }

  sws_ctx_ =
      sws_getContext(codec_context_->width, codec_context_->height,
                     codec_context_->pix_fmt, 1920, 1080, AV_PIX_FMT_RGB24,
                     SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
  if (sws_ctx_ == nullptr) {
    PLOG(ERROR) << "Could not create sws context";
    return false;
  }

  while (av_read_frame(format_context_, packet_) >= 0) {
    if (packet_->stream_index != video_stream_index_) {
      av_packet_unref(packet_);
      PLOG(ERROR) << "Could not read frame";
      return false;
    }

    if (const auto ret = avcodec_send_packet(codec_context_, packet_);
        ret < 0) {
      PLOG(ERROR) << "avcodec_send_packet error";
      return false;
    }

    if (const auto ret = avcodec_receive_frame(codec_context_, frame_);
        ret != 0) {
      PLOG(ERROR) << "avcodec_receive_frame error";
      return false;
    }

    av_packet_unref(packet_);

    if (frame_->data == nullptr) {
      PLOG(ERROR) << "Frame data is null";
      return false;
    }

    auto* dst_data = frame_data_.data.data();
    constexpr int dst_linesize = 1920 * 3;

    if (sws_scale(sws_ctx_, frame_->data, frame_->linesize, 0,
                  codec_context_->height, &dst_data, &dst_linesize) < 0) {
      PLOG(ERROR) << "Could not scale frame";
      std::cerr << "Error: Failed to scale frame" << std::endl;
      return false;
    }

    frames_.push_back(frame_data_);
    frame_count_++;
  }

  (void)avcodec_send_packet(codec_context_, nullptr);

  while (avcodec_receive_frame(codec_context_, frame_) == 0) {
  }

  looped_ = true;
  current_frame_ = 0;

  (void)av_seek_frame(format_context_, video_stream_index_, 0,
                      AVSEEK_FLAG_BACKWARD);

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
bool VideoDecoder::has_data() { return looped_; }

std::optional<std::unique_ptr<models::FrameData>>
VideoDecoder::get_next_frame() {
  const models::FrameData& frame =
      frames_.at(static_cast<size_t>(current_frame_));

  current_frame_++;

  if (current_frame_ >= frame_count_) {
    current_frame_ = 0;
  }

  return std::make_unique<models::FrameData>(frame);
}

}  // namespace screen_controller::processing
