#include <csignal> 
#include "LockFreeMPSCLogger/LogMacro.hpp"
#include "Capture/VideoCapture.hpp"
#include "Demo/DemoTest.hpp"

int main() {
    std::unique_ptr<Demo::TestTrigger> demoTrigger = std::make_unique<Demo::TestTrigger>();
    while (true) {
        demoTrigger->RunAllTests();
        std::this_thread::sleep_for(std::chrono::seconds(10));
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