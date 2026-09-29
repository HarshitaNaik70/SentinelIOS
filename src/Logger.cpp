#include "Logger.h"
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

// Private Constructor
Logger::Logger() : m_log_file_path("logs/sentinel.log") {
    // Ensure logs/ directory exists
    if (!fs::exists("logs")) {
        fs::create_directory("logs");
    }

    m_log_file.open(m_log_file_path, std::ios::app);
    if (!m_log_file.is_open()) {
        std::cerr << "[Logger] Warning: Failed to open log file at " << m_log_file_path << std::endl;
    }
}

// Destructor closes log file handle
Logger::~Logger() {
    if (m_log_file.is_open()) {
        m_log_file.close();
    }
}

// Static Accessor to Singleton Instance
Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

// Configure custom log file path
void Logger::set_log_file(const std::string& filepath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_log_file.is_open()) {
        m_log_file.close();
    }
    m_log_file_path = filepath;
    m_log_file.open(m_log_file_path, std::ios::app);
}

// Converts enum LogLevel to String representation
std::string Logger::level_to_string(LogLevel level) {
    switch (level) {
        case LogLevel::INFO:     return "INFO";
        case LogLevel::WARNING:  return "WARN";
        case LogLevel::ERROR:    return "ERROR";
        case LogLevel::CRITICAL: return "CRIT";
        default:                 return "INFO";
    }
}

// Formats current system timestamp
std::string Logger::get_current_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_now{};

#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&tm_now, &time_t_now);
#else
    localtime_r(&time_t_now, &tm_now);
#endif

    std::ostringstream ss;
    ss << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// Thread-Safe Logging Implementation
void Logger::log(LogLevel level, const std::string& module, const std::string& message) {
    std::lock_guard<std::mutex> lock(m_mutex);

    std::string timestamp = get_current_timestamp();
    std::string level_str = level_to_string(level);

    std::string formatted_msg = "[" + timestamp + "] [" + level_str + "] [" + module + "] " + message;

    // Print to Standard Console Output
    std::cout << formatted_msg << std::endl;

    // Append to Log File
    if (m_log_file.is_open()) {
        m_log_file << formatted_msg << std::endl;
    }
}

void Logger::info(const std::string& module, const std::string& message) {
    log(LogLevel::INFO, module, message);
}

void Logger::warn(const std::string& module, const std::string& message) {
    log(LogLevel::WARNING, module, message);
}

void Logger::error(const std::string& module, const std::string& message) {
    log(LogLevel::ERROR, module, message);
}

void Logger::critical(const std::string& module, const std::string& message) {
    log(LogLevel::CRITICAL, module, message);
}
