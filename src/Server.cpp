#include "Server.h"
#include "Logger.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <iomanip>

#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

Server::Server(int port) : m_port(port) {}

Server::~Server() {
    stop();
}

// Starts the TCP server socket bind, listen, and accept loop thread
bool Server::start() {
    if (m_is_running.load()) {
        return true;
    }

#if defined(_WIN32) || defined(_WIN64)
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    // 1. Create IPv4 TCP Stream Socket
    m_server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_server_fd < 0) {
        Logger::getInstance().error("Server", "Failed to create TCP socket.");
        return false;
    }

    // 2. Set SO_REUSEADDR option to allow instant socket rebinding upon restart
    int opt = 1;
#if defined(_WIN32) || defined(_WIN64)
    setsockopt(m_server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
    setsockopt(m_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    // 3. Bind socket to target port (0.0.0.0:9090)
    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(m_port);

    if (bind(m_server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        Logger::getInstance().error("Server", "Failed to bind TCP socket to port " + std::to_string(m_port));
#if defined(_WIN32) || defined(_WIN64)
        closesocket(m_server_fd);
#else
        close(m_server_fd);
#endif
        return false;
    }

    // 4. Listen for incoming client connections (Backlog queue: 5)
    if (listen(m_server_fd, 5) < 0) {
        Logger::getInstance().error("Server", "Failed to listen on TCP socket.");
        return false;
    }

    m_is_running.store(true);
    Logger::getInstance().info("Server", "TCP Server listening on IPv4 0.0.0.0:" + std::to_string(m_port));

    // 5. Launch background thread to handle incoming accept() connections
    m_accept_thread = std::thread(&Server::accept_loop, this);
    return true;
}

// Accept Loop: Waits for incoming client connections
void Server::accept_loop() {
    while (m_is_running.load()) {
        struct sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(m_server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            if (!m_is_running.load()) break;
            continue;
        }

        std::string client_ip = inet_ntoa(client_addr.sin_addr);
        Logger::getInstance().info("Server", "Client connected from " + client_ip + ":" + std::to_string(ntohs(client_addr.sin_port)));

        // Spawn a detached worker thread for each connected client
        std::thread client_thread(&Server::handle_client, this, client_fd, client_ip);
        client_thread.detach();
    }
}

// Client Worker Thread: Reads command requests and sends responses
void Server::handle_client(int client_fd, std::string client_ip) {
    char buffer[1024];
    while (m_is_running.load()) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

        if (bytes_received <= 0) {
            Logger::getInstance().info("Server", "Client disconnected from " + client_ip);
            break;
        }

        std::string raw_command(buffer);
        // Trim whitespace and carriage returns
        raw_command.erase(raw_command.find_last_not_of("\r\n\t ") + 1);

        if (raw_command.empty()) continue;

        Logger::getInstance().info("Server", "[" + client_ip + "] Command Received: " + raw_command);

        std::string response = process_command(raw_command);
        send(client_fd, response.c_str(), static_cast<int>(response.length()), 0);
    }

#if defined(_WIN32) || defined(_WIN64)
    closesocket(client_fd);
#else
    close(client_fd);
#endif
}

// Command Processor: Maps string command to module data responses
std::string Server::process_command(const std::string& raw_command) {
    std::string cmd = raw_command;
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);

    std::ostringstream ss;
    std::lock_guard<std::mutex> lock(m_monitor_mutex);

    if (cmd == "GET_CPU") {
        double cpu = m_resource_monitor.calculate_cpu_usage();
        ss << "CPU_USAGE: " << std::fixed << std::setprecision(2) << cpu << "%\n";
    } else if (cmd == "GET_MEMORY") {
        MemoryStats mem = m_resource_monitor.get_memory_stats();
        ss << "RAM_TOTAL_MB: " << mem.total_ram_mb << "\n"
           << "RAM_USED_MB: " << mem.used_ram_mb << "\n"
           << "RAM_AVAIL_MB: " << mem.available_ram_mb << "\n"
           << "RAM_USAGE_PERCENT: " << mem.ram_usage_percent << "%\n";
    } else if (cmd == "GET_DISK") {
        DiskStats disk = m_resource_monitor.get_disk_stats("/");
        ss << "DISK_TOTAL_GB: " << disk.total_disk_gb << "\n"
           << "DISK_USED_GB: " << disk.used_disk_gb << "\n"
           << "DISK_FREE_GB: " << disk.free_disk_gb << "\n"
           << "DISK_USAGE_PERCENT: " << disk.disk_usage_percent << "%\n";
    } else if (cmd == "GET_KERNEL") {
        KernelVersionInfo kver = m_kernel_monitor.get_kernel_version_info();
        CpuHardwareInfo cpu_info = m_kernel_monitor.get_cpu_info();
        SystemInfo sys_info = m_kernel_monitor.get_system_info();

        ss << "KERNEL_RELEASE: " << kver.kernel_release << "\n"
           << "OS_DISTRO: " << kver.os_distro << "\n"
           << "HOSTNAME: " << sys_info.hostname << "\n"
           << "CPU_MODEL: " << cpu_info.model_name << "\n"
           << "CPU_CORES: " << cpu_info.num_cores << "\n"
           << "UPTIME_SECONDS: " << sys_info.uptime_seconds << "\n";
    } else if (cmd == "GET_PROCESSES") {
        ProcessStatsSummary stats = m_process_manager.get_process_stats_summary();
        ss << "TOTAL_PROCESSES: " << stats.total_processes << "\n"
           << "RUNNING: " << stats.running_count << "\n"
           << "SLEEPING: " << stats.sleeping_count << "\n"
           << "ZOMBIE: " << stats.zombie_count << "\n"
           << "STOPPED: " << stats.stopped_count << "\n";
    } else if (cmd == "GET_SYSTEM_STATUS") {
        double cpu = m_resource_monitor.calculate_cpu_usage();
        MemoryStats mem = m_resource_monitor.get_memory_stats();
        ProcessStatsSummary stats = m_process_manager.get_process_stats_summary();

        ss << "STATUS: OK\n"
           << "CPU_PERCENT: " << cpu << "%\n"
           << "RAM_PERCENT: " << mem.ram_usage_percent << "%\n"
           << "TOTAL_PIDS: " << stats.total_processes << "\n";
    } else {
        ss << "ERROR: Unknown command '" << raw_command << "'. Supported commands: GET_CPU, GET_MEMORY, GET_DISK, GET_KERNEL, GET_PROCESSES, GET_SYSTEM_STATUS\n";
    }

    return ss.str();
}

// Stops the TCP server and closes sockets
void Server::stop() {
    if (!m_is_running.exchange(false)) {
        return;
    }

    Logger::getInstance().info("Server", "Stopping TCP Server...");

#if defined(_WIN32) || defined(_WIN64)
    if (m_server_fd >= 0) closesocket(m_server_fd);
    WSACleanup();
#else
    if (m_server_fd >= 0) close(m_server_fd);
#endif

    m_server_fd = -1;

    if (m_accept_thread.joinable()) {
        m_accept_thread.join();
    }

    Logger::getInstance().info("Server", "TCP Server stopped.");
}
