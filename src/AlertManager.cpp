#include "AlertManager.h"
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

AlertManager::AlertManager(const std::string& log_file_path)
    : m_log_file_path(log_file_path) {
    m_log_file.open(m_log_file_path, std::ios::app);
    if (!m_log_file.is_open()) {
        std::cerr << "[AlertManager] Warning: Could not open log file " << m_log_file_path << std::endl;
    }
}

AlertManager::~AlertManager() {
    if (m_log_file.is_open()) {
        m_log_file.close();
    }
}

std::string AlertManager::level_to_string(LogLevel level) {
    switch (level) {
        case LogLevel::INFO:     return "INFO";
        case LogLevel::WARNING:  return "WARN";
        case LogLevel::CRITICAL: return "CRIT";
        default:                 return "INFO";
    }
}

std::string AlertManager::get_current_timestamp() {
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

void AlertManager::log(LogLevel level, const std::string& module, const std::string& message) {
    std::lock_guard<std::mutex> lock(m_log_mutex);
    std::string timestamp = get_current_timestamp();
    std::string lvl_str = level_to_string(level);

    std::string log_line = "[" + timestamp + "] [" + lvl_str + "] [" + module + "] " + message;

    // Print to Console
    std::cout << log_line << std::endl;

    // Write to Log File
    if (m_log_file.is_open()) {
        m_log_file << log_line << std::endl;
    }
}

void AlertManager::info(const std::string& module, const std::string& message) {
    log(LogLevel::INFO, module, message);
}

void AlertManager::warn(const std::string& module, const std::string& message) {
    log(LogLevel::WARNING, module, message);
}

void AlertManager::error(const std::string& module, const std::string& message) {
    log(LogLevel::CRITICAL, module, message);
}
