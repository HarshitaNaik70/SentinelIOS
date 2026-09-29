#ifndef CLIENT_H
#define CLIENT_H

#include <string>

/**
 * @class Client
 * @brief TCP Monitoring Client connecting to SentinelOS server (port 9090) to query system telemetry.
 */
class Client {
private:
    std::string m_server_ip;
    int m_port;
    int m_socket_fd{-1};
    bool m_is_connected{false};

public:
    explicit Client(const std::string& ip = "127.0.0.1", int port = 9090);
    ~Client();

    // Initiates TCP socket connection to SentinelOS server
    bool connect_to_server();

    // Closes socket connection
    void disconnect();

    // Connection status accessor
    bool is_connected() const { return m_is_connected; }

    // Transmits command string and reads server response stream
    std::string send_command(const std::string& command);

    // Displays interactive terminal menu for operator
    void run_interactive_menu();
};

#endif // CLIENT_H
