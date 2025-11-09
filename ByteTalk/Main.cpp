#include <csignal> 
#include "LockFreeMPSCLogger/LogMacro.hpp"
#include "Capture/VideoCapture.hpp"

int main() {
    std::unique_ptr<Capture::VideoCapture> videoCapture = 
        std::make_unique<Capture::VideoCapture>("/dev/video0", 640, 480, 30);
    while (true) {
        std::vector<uint8_t> frameData;
        if (videoCapture->readFrame(frameData)) {
            LOG_INFO("Captured frame of size: " << frameData.size());
        } else {
            LOG_ERROR("Failed to capture frame.");
        }
    }
    LOG_INFO("CHATProcessor started. waiting for termination signal...");
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    sigprocmask(SIG_BLOCK, &mask, nullptr);
    int sig{0};
    sigwait(&mask, &sig);
    return 0;
}