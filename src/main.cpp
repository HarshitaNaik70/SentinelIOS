#include <iostream>
#include "App.h"

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "   " << Sentinel::PLATFORM_NAME << " v" << Sentinel::VERSION << std::endl;
    std::cout << "   Autonomous Linux Health Monitoring & Self-Healing      " << std::endl;
    std::cout << "==========================================================" << std::endl;

    Sentinel::SentinelApp app;
    Sentinel::Status status = app.initialize();

    if (status == Sentinel::Status::SUCCESS) {
        app.run();
        app.shutdown();
    } else {
        std::cerr << "[Main] Error initializing application." << std::endl;
        return static_cast<int>(status);
    }

    std::cout << "==========================================================" << std::endl;
    return static_cast<int>(Sentinel::Status::SUCCESS);
}
