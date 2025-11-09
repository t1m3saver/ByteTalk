#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <memory>
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
}

struct AVFormatContext;
struct AVCodecContext;
struct AVFrame;
struct SwsContext;

namespace Capture {
struct FFmpegDeleters {
    void operator()(AVFormatContext* ctx) const {
        if (ctx) {
            avformat_close_input(&ctx);
        }
    }
    void operator()(AVCodecContext* ctx) const {
        if (ctx) {
            avcodec_free_context(&ctx);
        }
    }
    void operator()(AVFrame* frame) const {
        if (frame) {
            av_frame_free(&frame);
        }
    }
    void operator()(SwsContext* ctx) const {
        if (ctx) {
            sws_freeContext(ctx);
        }
    }
};
class VideoCapture {
public:
    VideoCapture(std::string deviceName, int width = 640, int height = 480, int fps = 30);
    ~VideoCapture();

    bool readFrame(std::vector<uint8_t>& outRGB);

private:
    void openDevice();
    void getAllDevices(const AVInputFormat* ifmt, std::vector<std::string>& deviceNames);
    bool openSingleDevice(const AVInputFormat* ifmt, const std::string& devName);

private:
    std::unique_ptr<AVFormatContext, FFmpegDeleters> fmtCtx_;
    std::unique_ptr<AVCodecContext, FFmpegDeleters> codecCtx_;
    std::unique_ptr<SwsContext, FFmpegDeleters> swsCtx_;
    std::unique_ptr<AVFrame, FFmpegDeleters> frame_;

    int videoStreamIndex_;
    std::string deviceName_;
    int width_;
    int height_;
    int fps_;
};
}