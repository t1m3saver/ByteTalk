#pragma once
#include <string>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/file.h>
}

namespace Demo {
class TestTrigger {
public:
    void RunAllTests() {
        // 
        std::string testFile = "/home/liu/code/ByteTalk/test.mp4";
        TestAVIOReading(testFile);
    }
private:
    void TestAVIOReading(const std::string& fileName);
};
}