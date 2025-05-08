//
// Created by marce on 5/6/2025.
//

#include "video_decoder.h"

#include <fstream>
#include <iostream>
#include <vector>

namespace screen_controller::processing {

VideoDecoder::VideoDecoder(const std::string_view path)
    : video_data_{},
      image_data_{
          .data = std::vector<uint8_t>(1920 * 1080 * 3, 0),
          .width = 1920,
          .height = 1080,
          .channels = 3,
      },
      path_(path),
      codec_context_(nullptr),
      format_context_(nullptr),
      video_stream_index_(-1),
      frame_(nullptr),
      packet_(nullptr),
      sws_ctx_(nullptr) {}

bool VideoDecoder::init() {
  if (avformat_open_input(&format_context_, path_.data(), nullptr, nullptr) <
      0) {
    std::cerr << "FileProcessor::process_file: avformat_open_input error"
              << std::endl;

    return false;
  }

  if (avformat_find_stream_info(format_context_, nullptr) < 0) {
    std::cerr << "FileProcessor::process_file: avformat_find_stream_info error"
              << std::endl;
    return false;
  }

  for (unsigned int i = 0U; i < format_context_->nb_streams; ++i) {
    if (format_context_->streams[i]->codecpar->codec_type ==
        AVMEDIA_TYPE_VIDEO) {
      video_stream_index_ = i;
      break;
    }
  }

  if (video_stream_index_ == -1) {
    std::cerr << "Error: Could not find a video stream in the file."
              << std::endl;
    return false;
  }
  const AVCodec* codec = avcodec_find_decoder(
      format_context_->streams[video_stream_index_]->codecpar->codec_id);

  if (codec == nullptr) {
    std::cerr << "Error: Failed to find decoder for codec ID: " << std::endl;
    return false;
  }

  codec_context_ = avcodec_alloc_context3(codec);

  if (codec_context_ == nullptr) {
    std::cerr << "Could not allocate codec context" << std::endl;
    return false;
  }

  if (avcodec_parameters_to_context(
          codec_context_,
          format_context_->streams[video_stream_index_]->codecpar) < 0) {
    std::cerr << "Could not copy codec parameters" << std::endl;
    return false;
  }

  if (avcodec_open2(codec_context_, codec, nullptr) < 0) {
    std::cerr << "Could not open codec" << std::endl;
    return false;
  }

  frame_ = av_frame_alloc();
  packet_ = av_packet_alloc();

  if (frame_ == nullptr || packet_ == nullptr) {
    std::cerr << "Could not allocate frame or packet" << std::endl;
    return false;
  }

  sws_ctx_ =
      sws_getContext(codec_context_->width, codec_context_->height,
                     codec_context_->pix_fmt, 1920, 1080, AV_PIX_FMT_RGB24,
                     SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
  if (sws_ctx_ == nullptr) {
    std::cerr << "Could not create sws context" << std::endl;
    return false;
  }
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

std::optional<std::shared_ptr<models::FrameData>>
VideoDecoder::get_next_frame() {
  if (looped_) {
    models::FrameData frame = video_data_.at(current_frame_);
    current_frame_++;
    if (current_frame_ >= frame_count_) {
      current_frame_ = 0;
    }

    return std::make_shared<models::FrameData>(frame);
  }
  if (av_read_frame(format_context_, packet_) < 0) {
    avcodec_send_packet(codec_context_, nullptr);

    while (avcodec_receive_frame(codec_context_, frame_) == 0) {
    }
    looped_ = true;
    av_seek_frame(format_context_, video_stream_index_, 0,
                  AVSEEK_FLAG_BACKWARD);
    avcodec_flush_buffers(codec_context_);

    return std::nullopt;
  }

  if (packet_->stream_index != video_stream_index_) {
    av_packet_unref(packet_);
    return std::nullopt;
  }

  if (const auto ret = avcodec_send_packet(codec_context_, packet_); ret < 0) {
    return std::nullopt;
  }

  if (const auto ret = avcodec_receive_frame(codec_context_, frame_);
      ret != 0) {
    return std::nullopt;
  }

  av_packet_unref(packet_);

  if (frame_->data == nullptr) {
    return std::nullopt;
  }

  uint8_t* dst_data[] = {image_data_.data.data(), nullptr, nullptr, nullptr};
  int dst_linesize[] = {1920 * 3, 0, 0, 0};
  if (const auto ret =
          sws_scale(sws_ctx_, frame_->data, frame_->linesize, 0,
                    codec_context_->height, dst_data, dst_linesize);
      ret < 0) {
    std::cerr << "Error: Failed to scale frame" << std::endl;
    return std::nullopt;
  }
  video_data_.emplace_back(image_data_);
  frame_count_++;
  return std::make_shared<models::FrameData>(image_data_);
}

}  // namespace screen_controller::processing
