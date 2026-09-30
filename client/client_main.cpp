#include <iostream>
#include "Client.h"

int main(int argc, char* argv[]) {
    std::string ip = "127.0.0.1";
    int port = 9090;

    if (argc > 1) {
        ip = argv[1];
    }
    if (argc > 2) {
        port = std::stoi(argv[2]);
    }

    std::cout << "==========================================================" << std::endl;
    std::cout << "        SentinelOS: TCP Telemetry Client                  " << std::endl;
    std::cout << "==========================================================" << std::endl;
    std::cout << "[Target Server] " << ip << ":" << port << std::endl;

    Client client(ip, port);
    client.run_interactive_menu();

    std::cout << "[Client] Application exited cleanly." << std::endl;
    return 0;
}
