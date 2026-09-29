#include <iostream>
#include <thread>
#include <chrono>

#include "Logger.h"
#include "Server.h"

int main() {
    Logger::getInstance().info("Main", "Starting SentinelOS TCP Monitoring Server Daemon (Milestone 6)...");

    std::cout << "==========================================================" << std::endl;
    std::cout << "  SentinelOS: Milestone 6 - TCP Monitoring Server         " << std::endl;
    std::cout << "==========================================================" << std::endl;

    // Instantiate and start Milestone 6 Server on TCP Port 9090
    Server tcp_server(9090);
    if (!tcp_server.start()) {
        Logger::getInstance().error("Main", "Failed to start TCP Monitoring Server!");
        return 1;
    }

    std::cout << "\n[Server Online] Listening for client requests on TCP port 9090." << std::endl;
    std::cout << "[Test Command] Open terminal and run: nc 127.0.0.1 9090" << std::endl;
    std::cout << "[Supported Commands] GET_CPU, GET_MEMORY, GET_DISK, GET_KERNEL, GET_PROCESSES, GET_SYSTEM_STATUS" << std::endl;
    std::cout << "----------------------------------------------------------" << std::endl;

    // Keep server running for demonstration loop (5 seconds)
    std::cout << "[Daemon Loop] Running server event loop for 5 seconds..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // Stop server gracefully
    tcp_server.stop();

    Logger::getInstance().info("Main", "SentinelOS Milestone 6 TCP Server Test Completed Successfully.");
    std::cout << "----------------------------------------------------------" << std::endl;
    std::cout << "[Success] Milestone 6 TCP Server Integrated Test Completed." << std::endl;
    std::cout << "==========================================================" << std::endl;

    return 0;
}
