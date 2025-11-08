#include "VideoCapture.hpp"
#include <stdexcept>
#include <iostream>
#include "LockFreeMPSCLogger/LogMacro.hpp"

extern "C" {
#include "libavdevice/avdevice.h"
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

namespace Capture {
VideoCapture::VideoCapture(int deviceIndex, int width, int height, int fps)
    : deviceIndex_(deviceIndex), width_(width), height_(height), fps_(fps) 
{
    try {
        avdevice_register_all();
        openDevice();
    } CATCH_DEFAULT
    
}

bool VideoCapture::readFrame(std::vector<uint8_t>& outRGB) {
    AVPacket pkt;
    av_packet_unref(&pkt);
    pkt.data = nullptr;
    pkt.size = 0;

    while (av_read_frame(fmtCtx_.get(), &pkt) >= 0) {
        if (pkt.stream_index == videoStreamIndex_) {
            int ret = avcodec_send_packet(codecCtx_.get(), &pkt);
            if (ret < 0) {
                av_packet_unref(&pkt);
                continue;
            }

            ret = avcodec_receive_frame(codecCtx_.get(), frame_.get());
            if (ret == 0) {
                outRGB.resize(width_ * height_ * 3);
                uint8_t* dstData[1] = { outRGB.data() };
                int dstLinesize[1] = { width_ * 3 };
                sws_scale(swsCtx_.get(), frame_->data, frame_->linesize, 0, height_, dstData, dstLinesize);
                av_packet_unref(&pkt);
                return true;
            }
        }
        av_packet_unref(&pkt);
    }
    return false;
}

void VideoCapture::openDevice() {
    const AVInputFormat* ifmt = av_find_input_format("v4l2");
    if (!ifmt) throw std::runtime_error("Cannot find v4l2 input format");

    std::string devName = "/dev/video" + std::to_string(deviceIndex_);
    AVFormatContext* tmpFmt = nullptr;
    if (avformat_open_input(&tmpFmt, devName.c_str(), ifmt, nullptr) != 0)
        throw std::runtime_error("Cannot open video device");
    fmtCtx_.reset(tmpFmt);  // 使用智能指针管理

    if (avformat_find_stream_info(fmtCtx_.get(), nullptr) < 0)
        throw std::runtime_error("Cannot find stream info");

    videoStreamIndex_ = -1;
    for (unsigned i = 0; i < fmtCtx_->nb_streams; ++i) {
        if (fmtCtx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex_ = i;
            break;
        }
    }
    if (videoStreamIndex_ == -1)
        throw std::runtime_error("Cannot find video stream");

    const AVCodec* codec = avcodec_find_decoder(fmtCtx_->streams[videoStreamIndex_]->codecpar->codec_id);
    if (!codec) throw std::runtime_error("Cannot find codec");

    AVCodecContext* tmpCodecCtx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(tmpCodecCtx, fmtCtx_->streams[videoStreamIndex_]->codecpar);
    if (avcodec_open2(tmpCodecCtx, codec, nullptr) < 0)
        throw std::runtime_error("Cannot open codec");
    codecCtx_.reset(tmpCodecCtx);

    AVFrame* tmpFrame = av_frame_alloc();
    frame_.reset(tmpFrame);

    SwsContext* tmpSws = sws_getContext(codecCtx_->width, codecCtx_->height, codecCtx_->pix_fmt,
                                        width_, height_, AV_PIX_FMT_RGB24,
                                        SWS_BILINEAR, nullptr, nullptr, nullptr);
    swsCtx_.reset(tmpSws);
}

} // namespace Capture
