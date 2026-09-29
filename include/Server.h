#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <atomic>
#include "ThreadCompat.h"
#include "ResourceMonitor.h"
#include "KernelMonitor.h"
#include "ProcessManager.h"

/**
 * @class Server
 * @brief Multithreaded TCP Monitoring Server exposing SentinelOS telemetry over TCP port 9090.
 */
class Server {
private:
    int m_port;
    int m_server_fd{-1};
    std::atomic<bool> m_is_running{false};
    std::thread m_accept_thread;

    // Subsystem handles for telemetry retrieval
    ResourceMonitor m_resource_monitor;
    KernelMonitor m_kernel_monitor;
    ProcessManager m_process_manager;
    std::mutex m_monitor_mutex;

    // Internal socket connection loops
    void accept_loop();
    void handle_client(int client_fd, std::string client_ip);
    std::string process_command(const std::string& raw_command);

public:
    explicit Server(int port = 9090);
    ~Server();

    // Starts the TCP server accept loop on background thread
    bool start();

    // Stops the TCP server and closes server socket
    void stop();

    // Checks server operational status
    bool is_running() const { return m_is_running.load(); }
};

#endif // SERVER_H
