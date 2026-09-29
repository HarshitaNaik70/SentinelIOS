#include "Logger.h"
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <sys/stat.h>

#if defined(_WIN32)
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#define MKDIR(path) mkdir(path, 0755)
#endif

// Private Constructor
Logger::Logger() : m_log_file_path("logs/sentinel.log") {
    struct stat st;
    if (stat("logs", &st) != 0) {
        MKDIR("logs");
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

// Singleton Instance Accessor
Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

// Converts LogLevel enum to string representation
std::string Logger::level_to_string(LogLevel level) {
    switch (level) {
        case LogLevel::INFO:     return "INFO";
        case LogLevel::WARNING:  return "WARNING";
        case LogLevel::ERROR:    return "ERROR";
        case LogLevel::CRITICAL: return "CRITICAL";
        default:                 return "UNKNOWN";
    }
}

// Generates current wall-clock timestamp string
std::string Logger::get_current_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_now{};

#if defined(_MSC_VER)
    localtime_s(&tm_now, &time_t_now);
#elif defined(_WIN32)
    std::tm* tm_ptr = std::localtime(&time_t_now);
    if (tm_ptr) tm_now = *tm_ptr;
#else
    localtime_r(&time_t_now, &tm_now);
#endif

    std::ostringstream ss;
    ss << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// Sets custom target log file path
void Logger::set_log_file(const std::string& filepath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_log_file.is_open()) {
        m_log_file.close();
    }
    m_log_file_path = filepath;
    m_log_file.open(m_log_file_path, std::ios::app);
}

// Core Log Implementation
void Logger::log(LogLevel level, const std::string& module, const std::string& message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::string timestamp = get_current_timestamp();
    std::string level_str = level_to_string(level);

    // Formatted Log Record: [TIMESTAMP] [LEVEL] [MODULE] - Message
    std::string log_entry = "[" + timestamp + "] [" + level_str + "] [" + module + "] - " + message;

    // 1. Output to Console
    std::cout << log_entry << std::endl;

    // 2. Output to persistent log file
    if (m_log_file.is_open()) {
        m_log_file << log_entry << std::endl;
        m_log_file.flush();
    }
}

// Helper methods for log severity levels
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
