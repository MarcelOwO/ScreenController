//
// Created by marce on 5/6/2025.
//

#include "video_decoder.h"

#include <iostream>
#include <vector>

namespace screen_controller::processing {

VideoDecoder::VideoDecoder(std::string_view path) {
  if (!init()) {
    std::cerr << "Failed to initialize video decoder" << std::endl;
  }
}
bool VideoDecoder::init() { return true; }

bool VideoDecoder::process_video(const std::string_view path) {
  AVFormatContext* format_context = nullptr;

  if (avformat_open_input(&format_context, path.data(), nullptr, nullptr) < 0) {
    std::cerr << "FileProcessor::process_file: avformat_open_input error"
              << std::endl;
    return false;
  }

  if (avformat_find_stream_info(format_context, nullptr) < 0) {
    std::cerr << "FileProcessor::process_file: avformat_find_stream_info error"
              << std::endl;
    avformat_close_input(&format_context);
    return false;
  }

  int video_stream_index = -1;

  for (int i = 0; i < format_context->nb_streams; ++i) {
    if (format_context->streams[i]->codecpar->codec_type ==
        AVMEDIA_TYPE_VIDEO) {
      video_stream_index = i;
      break;
    }
  }

  if (video_stream_index == -1) {
    std::cerr << "Error: Could not find a video stream in the file."
              << std::endl;
    avformat_close_input(&format_context);
    return false;
  }

  const AVCodec* codec = avcodec_find_decoder(
      format_context->streams[video_stream_index]->codecpar->codec_id);
  if (!codec) {
    std::cerr << "Error: Failed to find decoder for codec ID: " << std::endl;
    avformat_close_input(&format_context);
    return false;
  }

  AVCodecContext* codec_context = avcodec_alloc_context3(codec);
  if (!codec_context) {
    std::cerr << "Could not allocate codec context" << std::endl;
    avformat_close_input(&format_context);
    return {};
  }

  if (avcodec_parameters_to_context(
          codec_context,
          format_context->streams[video_stream_index]->codecpar) < 0) {
    std::cerr << "Could not copy codec parameters" << std::endl;
    avcodec_free_context(&codec_context);
    avformat_close_input(&format_context);
    return {};
  }

  if (avcodec_open2(codec_context, codec, nullptr) < 0) {
    std::cerr << "Could not open codec" << std::endl;
    avcodec_free_context(&codec_context);
    avformat_close_input(&format_context);
    return {};
  }

  AVFrame* frame = av_frame_alloc();
  AVPacket* packet = av_packet_alloc();

  if (!frame || !packet) {
    std::cerr << "Could not allocate frame or packet" << std::endl;
    avcodec_free_context(&codec_context);
    avformat_close_input(&format_context);
    return {};
  }

  while (av_read_frame(format_context, packet) >= 0) {
    if (packet->stream_index != video_stream_index) {
      continue;
    }
    if (!decode(codec_context, frame, packet)) {
      return false;
    }
    av_packet_unref(packet);
  }

  av_frame_free(&frame);
  av_packet_free(&packet);
  avcodec_free_context(&codec_context);
  avformat_close_input(&format_context);
  return true;
}

bool VideoDecoder::decode(AVCodecContext* dec_ctx, AVFrame* frame,
                          const AVPacket* pkt) {
  std::vector<uint8_t> rgb_data;

  int ret = avcodec_send_packet(dec_ctx, pkt);
  if (ret < 0) {
    std::cerr << "Error sending a packet for decoding\n";
    return {};
  }

  constexpr AVPixelFormat target_format = AV_PIX_FMT_RGB24;

  SwsContext* sws_ctx = nullptr;

  while (true) {
    constexpr int target_height = 1080;
    constexpr int target_width = 1920;
    ret = avcodec_receive_frame(dec_ctx, frame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
      break;
    }
    if (ret < 0) {
      std::cerr << "Error during decoding\n";
      return false;
    }

    AVFrame* resized_frame = av_frame_alloc();
    if (!resized_frame) {
      std::cerr << "Failed to allocate resized frame\n";
      return false;
    }

    resized_frame->format = target_format;
    resized_frame->width = target_width;
    resized_frame->height = target_height;

    ret = av_frame_get_buffer(resized_frame, 32);
    if (ret < 0) {
      std::cerr << "Could not allocate buffer for resized frame\n";
      av_frame_free(&resized_frame);
      return false;
    }

    if (!sws_ctx) {
      sws_ctx = sws_getContext(frame->width, frame->height,
                               static_cast<AVPixelFormat>(frame->format),
                               target_width, target_height, target_format,
                               SWS_BILINEAR, nullptr, nullptr, nullptr);
      if (!sws_ctx) {
        std::cerr << "Could not initialize the conversion context\n";
        av_frame_free(&resized_frame);
        return false;
      }
    }

    (void)sws_scale(sws_ctx, frame->data, frame->linesize, 0, frame->height,
                    resized_frame->data, resized_frame->linesize);

    constexpr int row_size = target_width * 3;
    for (int y = 0; y < target_height; ++y) {
      uint8_t* row_ptr =
          resized_frame->data[0] + y * resized_frame->linesize[0];
      (void)rgb_data.insert(rgb_data.end(), row_ptr, row_ptr + row_size);
    }

    av_frame_free(&resized_frame);
  }

  if (sws_ctx) {
    sws_freeContext(sws_ctx);
  }

  return true;
}
}  // namespace screen_controller::processing