#include "DemoTest.hpp"
#include "LockFreeMPSCLogger/LogMacro.hpp"

namespace Demo {
struct buffer_data {
    uint8_t *ptr;
    size_t size; ///< size left in the buffer
};
 
static int read_packet(void *opaque, uint8_t *buf, int buf_size)
{
    buffer_data *bd = (buffer_data *)opaque;
    buf_size = FFMIN(buf_size, bd->size);
    if (!buf_size)
        return AVERROR_EOF;
    LOG_INFO("read_packet called: " << "buf_size=" << buf_size << " bd->size=" << bd->size);
 
    memcpy(buf, bd->ptr, buf_size);
    bd->ptr  += buf_size;
    bd->size -= buf_size;
 
    return buf_size;
}
/**
 * 这是一个使用FFmpeg AVIO机制从内存缓冲区读取媒体数据的示例，实现了不通过文件系统直接处理媒体数据。
 */
void TestTrigger::TestAVIOReading(const std::string& fileName)
{
    LOG_INFO("TestAVIOReading called with file: " << fileName);
    AVFormatContext *fmt_ctx = NULL;
    AVIOContext *avio_ctx = NULL;
    uint8_t *buffer = NULL, *avio_ctx_buffer = NULL;
    size_t buffer_size, avio_ctx_buffer_size = 4096;
    int ret = 0;
    struct buffer_data bd = { 0 };
    int video_stream_index = -1;
    int audio_stream_index = -1;
    int packet_count = 0;
    int max_packets = 50; // 限制读取的数据包数量，避免日志过多
    AVPacket *pkt = NULL;
    const char* input_filename = fileName.c_str();
 
    /* slurp file content into buffer */
    ret = av_file_map(input_filename, &buffer, &buffer_size, 0, NULL); // 将整个文件读入内存缓冲区, 这个函数需要常规文件，不能是设备文件或管道
    if (ret < 0) {
        LOG_ERROR("Could not open file: " << input_filename);
        goto end;
    }
 
    /* fill opaque structure used by the AVIOContext read callback */
    bd.ptr  = buffer;
    bd.size = buffer_size;
 
    avio_ctx_buffer = static_cast<uint8_t*>(av_malloc(avio_ctx_buffer_size)); // 分配AVIO缓冲区
    if (!avio_ctx_buffer) {
        LOG_ERROR("Could not allocate AVIO context buffer");
        ret = AVERROR(ENOMEM);
        goto end;
    }
    // 创建AVIO上下文，关键参数：
    // - 缓冲区 & 大小
    // - write_flag=0 (只读)
    // - opaque=&bd (传递给回调函数的自定义数据)
    // - read_packet (读取回调函数)
    avio_ctx = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size,
                                  0, &bd, &read_packet, NULL, NULL);
    if (!avio_ctx) {
        LOG_ERROR("Could not allocate AVIO context");
        ret = AVERROR(ENOMEM);
        goto end;
    }
    if (!(fmt_ctx = avformat_alloc_context())) { 
        LOG_ERROR("Could not allocate format context");
        ret = AVERROR(ENOMEM);
        goto end;
    }
    fmt_ctx->pb = avio_ctx; // 将AVIO上下文关联到格式上下文
 
    ret = avformat_open_input(&fmt_ctx, NULL, NULL, NULL); // 打开输入（注意：这里文件名传NULL，因为数据来自自定义AVIO）
    if (ret < 0) {
        LOG_ERROR("Could not open input");
        goto end;
    }
 
    ret = avformat_find_stream_info(fmt_ctx, NULL); // 获取流信息
    if (ret < 0) {
        LOG_ERROR("Could not find stream information.");
        goto end;
    }
    for (unsigned int i = 0; i < fmt_ctx->nb_streams; i++) {
        AVStream *stream = fmt_ctx->streams[i];
        AVCodecParameters *codecpar = stream->codecpar;
        
        LOG_INFO("Stream #" << i << ": "
                << "type=" << av_get_media_type_string(codecpar->codec_type) << ", "
                << "codec=" << avcodec_get_name(codecpar->codec_id) << ", "
                << "duration=" << stream->duration << ", "
                << "time_base=" << stream->time_base.num << "/" << stream->time_base.den);
    }
    // 查找视频流
    video_stream_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (video_stream_index >= 0) {
        AVStream *video_stream = fmt_ctx->streams[video_stream_index];
        LOG_INFO("Found video stream: " << video_stream_index 
                << ", width=" << video_stream->codecpar->width 
                << ", height=" << video_stream->codecpar->height);
    }

    // 查找音频流
    audio_stream_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (audio_stream_index >= 0) {
        AVStream *audio_stream = fmt_ctx->streams[audio_stream_index];
    
        char channel_layout[64] = {0};
        av_channel_layout_describe(&audio_stream->codecpar->ch_layout, channel_layout, sizeof(channel_layout));
        LOG_INFO("Found audio stream: " << audio_stream_index 
             << ", sample_rate=" << audio_stream->codecpar->sample_rate 
             << ", channels=" << audio_stream->codecpar->ch_layout.nb_channels
             << ", channel_layout=" << channel_layout);
    }
    pkt = av_packet_alloc();
    if (!pkt) {
        LOG_ERROR("Could not allocate packet");
        goto end;
    }

    while (packet_count < max_packets) {
        ret = av_read_frame(fmt_ctx, pkt);
        if (ret < 0) {
            if (ret == AVERROR_EOF) {
                LOG_INFO("End of file reached");
            } else {
                
                LOG_ERROR("Error reading packet: ");
            }
            break;
        }
        
        LOG_INFO("Read packet[" << packet_count << "]: stream_index=" << pkt->stream_index 
                << ", size=" << pkt->size 
                << ", pts=" << pkt->pts 
                << ", dts=" << pkt->dts);
        
        av_packet_unref(pkt);
        packet_count++;
    }

    av_packet_free(&pkt);
    
 
end:
    avformat_close_input(&fmt_ctx);
 
    /* note: the internal buffer could have changed, and be != avio_ctx_buffer */
    if (avio_ctx)
        av_freep(&avio_ctx->buffer);
    avio_context_free(&avio_ctx);
 
    av_file_unmap(buffer, buffer_size);
 
    if (ret < 0) {
        LOG_ERROR("Error occurred, ret < 0");
        return ;
    }
    LOG_INFO("TestAVIOReading completed successfully.");
    return;
}
}