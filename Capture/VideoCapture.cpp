#include "VideoCapture.hpp"
#include <iostream>
#include "LockFreeMPSCLogger/LogMacro.hpp"

extern "C" {
#include "libavdevice/avdevice.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
}

namespace Capture {

VideoCapture::VideoCapture(std::string deviceName = "all", int width, int height, int fps)
    : deviceName_(std::move(deviceName)), width_(width), height_(height), fps_(fps) 
{
    try {
        LOG_INFO("Initializing VideoCapture with device: " << deviceName_);
        avdevice_register_all();
        openDevice();
    } CATCH_DEFAULT
}

VideoCapture::~VideoCapture() = default;

bool VideoCapture::readFrame(std::vector<uint8_t>& outRGB) {
    try {
        AVPacket pkt;
        av_init_packet(&pkt);
        pkt.data = nullptr;
        pkt.size = 0;
        if (fmtCtx_ == nullptr || codecCtx_ == nullptr || frame_ == nullptr || swsCtx_ == nullptr) {
            LOG_ERROR("VideoCapture not properly initialized.");
            return false;
        }
        while (av_read_frame(fmtCtx_.get(), &pkt) >= 0) {
            if (pkt.stream_index == videoStreamIndex_) {
                int ret = avcodec_send_packet(codecCtx_.get(), &pkt);
                if (ret < 0) {
                    LOG_ERROR("Failed to send packet to decoder: " << ret);
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
                } else if (ret != AVERROR(EAGAIN)) {
                    LOG_ERROR("Decoder receive frame failed: " << ret);
                }
            }
            av_packet_unref(&pkt);
        }
    } CATCH_DEFAULT
    return false;
}

void VideoCapture::getAllDevices(const AVInputFormat* ifmt, std::vector<std::string>& deviceNames) 
{
    AVDeviceInfoList* device_list = nullptr;
    if (avdevice_list_input_sources(ifmt, nullptr, nullptr, &device_list) < 0 || !device_list) {
        LOG_ERROR("Cannot list video input devices.");
        return;
    }
    std::unique_ptr<AVDeviceInfoList, std::function<void(AVDeviceInfoList*)>> deviceListPtr(nullptr,
        [](AVDeviceInfoList* p) {
            if (p) avdevice_free_list_devices(&p);
        }
    );
    for (int i = 0; i < device_list->nb_devices; i++) {
        auto* device = deviceListPtr->devices[i];
        if (!device) {
            LOG_ERROR("Null device encountered.");
            continue;
        }
        LOG_INFO("Found device: " <<device->device_name << " - " << device->device_description);
        deviceNames.push_back(device->device_name);
    }
}

bool VideoCapture::openSingleDevice(const AVInputFormat* ifmt, const std::string& devName)
{
    AVFormatContext* tmpFmt = nullptr;
    if (avformat_open_input(&tmpFmt, devName.c_str(), ifmt, nullptr) != 0) {
        LOG_ERROR("Cannot open video device: " << devName);
        return false;
    }
    fmtCtx_.reset(tmpFmt);

    if (avformat_find_stream_info(fmtCtx_.get(), nullptr) < 0) {
        LOG_ERROR("Cannot find stream info in device: " << devName);
        return false;
    }

    videoStreamIndex_ = -1;
    for (unsigned i = 0; i < fmtCtx_->nb_streams; ++i) {
        if (fmtCtx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            LOG_INFO("AVMEDIA_TYPE_VIDEO stream found at index: " << i);
            videoStreamIndex_ = i;
            break;
        }
    }

    if (videoStreamIndex_ == -1) {
        LOG_ERROR("Cannot find video stream in device: " << devName);
        return false;
    }

    const AVCodec* codec = avcodec_find_decoder(fmtCtx_->streams[videoStreamIndex_]->codecpar->codec_id);
    if (!codec) {
        LOG_ERROR("Cannot find decoder for stream.");
        return false;
    }

    AVCodecContext* tmpCodecCtx = avcodec_alloc_context3(codec);
    if (!tmpCodecCtx) {
        LOG_ERROR("Failed to allocate codec context.");
        return false;
    }

    if (avcodec_parameters_to_context(tmpCodecCtx, fmtCtx_->streams[videoStreamIndex_]->codecpar) < 0) {
        LOG_ERROR("Failed to copy codec parameters.");
        avcodec_free_context(&tmpCodecCtx);
        return false;
    }

    if (avcodec_open2(tmpCodecCtx, codec, nullptr) < 0) {
        LOG_ERROR("Cannot open codec.");
        avcodec_free_context(&tmpCodecCtx);
        return false;
    }
    codecCtx_.reset(tmpCodecCtx);

    AVFrame* tmpFrame = av_frame_alloc();
    if (!tmpFrame) {
        LOG_ERROR("Failed to allocate frame.");
        return false;
    }
    frame_.reset(tmpFrame);

    SwsContext* tmpSws = sws_getContext(codecCtx_->width, codecCtx_->height, codecCtx_->pix_fmt,
                                        width_, height_, AV_PIX_FMT_RGB24,
                                        SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!tmpSws) {
        LOG_ERROR("Failed to initialize sws context.");
        return false;
    }
    swsCtx_.reset(tmpSws);
    LOG_INFO("Video device opened successfully: " << devName);
    return true;
}

void VideoCapture::openDevice() 
{
    const AVInputFormat* ifmt = av_find_input_format("v4l2");
    if (!ifmt) {
        LOG_ERROR("Cannot find input format: v4l2");
        return;
    }
    if (deviceName_.empty()) {
        LOG_ERROR("Device name is empty.");
        return;
    }
    std::vector<std::string> deviceNames;
    if (deviceName_ == "all") {
        getAllDevices(ifmt, deviceNames); 
    } else deviceNames.push_back(deviceName_);
    for (const auto& devName : deviceNames) {
        LOG_INFO("Attempting to open device: " << devName);
        openSingleDevice(ifmt, devName);
    }
}

} // namespace Capture
