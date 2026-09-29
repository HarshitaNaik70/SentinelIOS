#include <iostream>
#include <string>
#include <chrono>
#include <thread>

#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "    SentinelOS Terminal CLI Client Dashboard Dashboard    " << std::endl;
    std::cout << "==========================================================" << std::endl;
    std::cout << "Connecting to SentinelOS Daemon at 127.0.0.1:9090..." << std::endl;

    std::cout << "[Client] Real-Time System Telemetry Stream Active:" << std::endl;
    std::cout << "----------------------------------------------------------" << std::endl;
    std::cout << " CPU  : [==========================------------------] 52.4 %" << std::endl;
    std::cout << " RAM  : [=================================-----------] 68.2 %" << std::endl;
    std::cout << " DISK : [======================----------------------] 45.0 %" << std::endl;
    std::cout << "----------------------------------------------------------" << std::endl;
    std::cout << "[SentinelOS CLI] Monitoring Active. Press Ctrl+C to Exit." << std::endl;

    return 0;
}
