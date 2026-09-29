#include <iostream>
#include "Client.h"
#include "Logger.h"

int main(int argc, char* argv[]) {
    std::string ip = "127.0.0.1";
    int port = 9090;

    if (argc > 1) {
        ip = argv[1];
    }
    if (argc > 2) {
        port = std::stoi(argv[2]);
    }

    Logger::getInstance().info("ClientMain", "Starting SentinelOS Terminal CLI Client Application...");

    std::cout << "==========================================================" << std::endl;
    std::cout << "   SentinelOS: Milestone 7 - TCP Monitoring Client        " << std::endl;
    std::cout << "==========================================================" << std::endl;
    std::cout << "[Target Server] " << ip << ":" << port << std::endl;

    Client client(ip, port);
    client.run_interactive_menu();

    Logger::getInstance().info("ClientMain", "Client application exited cleanly.");
    return 0;
}
