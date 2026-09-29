#ifndef ALERT_MANAGER_H
#define ALERT_MANAGER_H

#include <string>
#include <fstream>
#include <mutex>

enum class LogLevel {
    INFO,
    WARNING,
    CRITICAL
};

class AlertManager {
private:
    std::string m_log_file_path;
    std::ofstream m_log_file;
    std::mutex m_log_mutex;

    std::string level_to_string(LogLevel level);
    std::string get_current_timestamp();

public:
    explicit AlertManager(const std::string& log_file_path = "sentinel.log");
    ~AlertManager();

    void log(LogLevel level, const std::string& module, const std::string& message);
    void info(const std::string& module, const std::string& message);
    void warn(const std::string& module, const std::string& message);
    void error(const std::string& module, const std::string& message);
};

#endif // ALERT_MANAGER_H
