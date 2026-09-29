#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <mutex>

// Log Severity Levels
enum class LogLevel {
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

/**
 * @class Logger
 * @brief Thread-safe Singleton Logger managing console output and persistent file logging (logs/sentinel.log).
 */
class Logger {
private:
    std::string m_log_file_path;
    std::ofstream m_log_file;
    std::mutex m_mutex;

    // Private Constructor for Singleton Pattern
    Logger();

    // Private Destructor
    ~Logger();

    // Helper functions
    std::string level_to_string(LogLevel level);
    std::string get_current_timestamp();

public:
    // Delete Copy Constructor and Assignment Operator to enforce Singleton
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // Static Accessor to Singleton Instance
    static Logger& getInstance();

    // Configures target log file path
    void set_log_file(const std::string& filepath);

    // Primary Logging Method
    void log(LogLevel level, const std::string& module, const std::string& message);

    // Convenience Log Helpers
    void info(const std::string& module, const std::string& message);
    void warn(const std::string& module, const std::string& message);
    void error(const std::string& module, const std::string& message);
    void critical(const std::string& module, const std::string& message);
};

#endif // LOGGER_H
