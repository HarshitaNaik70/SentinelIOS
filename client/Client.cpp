#if defined(_WIN32) || defined(_WIN64)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#if defined(_MSC_VER)
#pragma comment(lib, "ws2_32.lib")
#endif
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include "Client.h"
#include <iostream>
#include <sstream>
#include <cstring>

Client::Client(const std::string& ip, int port)
    : m_server_ip(ip), m_port(port) {}

Client::~Client() {
    disconnect();
}

// Connects socket to remote SentinelOS server
bool Client::connect_to_server() {
    if (m_is_connected) {
        return true;
    }

#if defined(_WIN32) || defined(_WIN64)
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    // 1. Create IPv4 TCP Stream Socket
    m_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket_fd < 0) {
        std::cerr << "[Client Error] Could not create TCP socket." << std::endl;
        return false;
    }

    // 2. Configure target Server IPv4 address and Port
    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(m_port);

#if defined(_WIN32) || defined(_WIN64)
    server_addr.sin_addr.s_addr = inet_addr(m_server_ip.c_str());
    if (server_addr.sin_addr.s_addr == INADDR_NONE && m_server_ip != "255.255.255.255") {
        std::cerr << "[Client Error] Invalid IP address format: " << m_server_ip << std::endl;
        return false;
    }
#else
    if (inet_pton(AF_INET, m_server_ip.c_str(), &server_addr.sin_addr) <= 0) {
        std::cerr << "[Client Error] Invalid IP address format: " << m_server_ip << std::endl;
        return false;
    }
#endif

    // 3. Initiate POSIX connect() handshake
    if (connect(m_socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "[Client Error] Server connection failed to " << m_server_ip << ":" << m_port << std::endl;
        disconnect();
        return false;
    }

    m_is_connected = true;
    std::cout << "[Client] Connected to SentinelOS Server (" << m_server_ip << ":" << m_port << ")" << std::endl;
    return true;
}

// Closes active socket connection
void Client::disconnect() {
    if (!m_is_connected && m_socket_fd < 0) {
        return;
    }

    std::cout << "[Client] Disconnecting from SentinelOS server..." << std::endl;

#if defined(_WIN32) || defined(_WIN64)
    if (m_socket_fd >= 0) closesocket(m_socket_fd);
    WSACleanup();
#else
    if (m_socket_fd >= 0) close(m_socket_fd);
#endif

    m_socket_fd = -1;
    m_is_connected = false;
}

// Transmits command and reads response string
std::string Client::send_command(const std::string& command) {
    if (!m_is_connected) {
        if (!connect_to_server()) {
            return "ERROR: Server connection unavailable.\n";
        }
    }

    std::string formatted_cmd = command + "\n";

    // 1. Transmit command bytes via POSIX send()
    if (send(m_socket_fd, formatted_cmd.c_str(), static_cast<int>(formatted_cmd.length()), 0) < 0) {
        std::cerr << "[Client Error] Failed to send command: " << command << std::endl;
        m_is_connected = false;
        return "ERROR: Network transmission failed.\n";
    }

    // 2. Read response bytes via POSIX recv()
    char buffer[2048];
    memset(buffer, 0, sizeof(buffer));

    int bytes_received = recv(m_socket_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received <= 0) {
        std::cout << "[Client] Server disconnected during response reading." << std::endl;
        m_is_connected = false;
        return "ERROR: Connection closed by server.\n";
    }

    std::string response(buffer);
    return response;
}

// Renders interactive console menu
void Client::run_interactive_menu() {
    if (!connect_to_server()) {
        std::cerr << "[Client] Could not establish connection to server. Exiting menu." << std::endl;
        return;
    }

    int choice = 0;
    while (choice != 7) {
        std::cout << "\n==========================================" << std::endl;
        std::cout << "       SentinelOS Client Dashboard        " << std::endl;
        std::cout << "==========================================" << std::endl;
        std::cout << "  1. CPU Usage" << std::endl;
        std::cout << "  2. Memory Usage" << std::endl;
        std::cout << "  3. Disk Usage" << std::endl;
        std::cout << "  4. Kernel Information" << std::endl;
        std::cout << "  5. Process Information" << std::endl;
        std::cout << "  6. System Status" << std::endl;
        std::cout << "  7. Exit" << std::endl;
        std::cout << "==========================================" << std::endl;
        std::cout << "Enter Choice (1-7): ";

        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            continue;
        }

        std::string cmd;
        switch (choice) {
            case 1: cmd = "GET_CPU"; break;
            case 2: cmd = "GET_MEMORY"; break;
            case 3: cmd = "GET_DISK"; break;
            case 4: cmd = "GET_KERNEL"; break;
            case 5: cmd = "GET_PROCESSES"; break;
            case 6: cmd = "GET_SYSTEM_STATUS"; break;
            case 7:
                std::cout << "[Client] Exiting client dashboard. Goodbye!" << std::endl;
                disconnect();
                return;
            default:
                std::cout << "[Client] Invalid option. Please select 1-7." << std::endl;
                continue;
        }

        std::cout << "\n--- Server Response ---" << std::endl;
        std::string response = send_command(cmd);
        std::cout << response;
        std::cout << "-----------------------" << std::endl;
    }
}
