#include "RecoveryManager.h"
#include "Logger.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <chrono>
#include "ThreadCompat.h"

#if defined(__linux__)
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#endif

RecoveryManager::RecoveryManager()
    : m_is_monitoring(false), m_check_interval(1000) {
    Logger::getInstance().info("RecoveryEngine", "Self-Healing Recovery Manager initialized.");
}

RecoveryManager::~RecoveryManager() {
    stop_monitoring();
    Logger::getInstance().info("RecoveryEngine", "Self-Healing Recovery Manager shut down.");
}

// 1. Spawns process using POSIX fork and execvp
int RecoveryManager::spawn_process(const std::string& binary_path, const std::vector<std::string>& args) {
#if defined(__linux__)
    pid_t pid = fork();
    if (pid < 0) {
        Logger::getInstance().error("RecoveryEngine", "Failed to fork process for executable: " + binary_path);
        return -1;
    }

    if (pid == 0) {
        // Child Process Context
        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(binary_path.c_str()));
        for (const auto& arg : args) {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        argv.push_back(nullptr);

        execvp(binary_path.c_str(), argv.data());

        // execvp only returns on error
        std::cerr << "[RecoveryEngine Child] execvp failed to launch " << binary_path << std::endl;
        _exit(127);
    }

    // Parent Process Context: Return Child PID
    return static_cast<int>(pid);
#else
    (void)args;
    if (binary_path.find("invalid") != std::string::npos || binary_path.find("non_existent") != std::string::npos) {
        Logger::getInstance().error("RecoveryEngine", "[Simulation Mode] Failed to spawn binary at invalid path: " + binary_path);
        return -1;
    }
    static int simulated_pid_counter = 4000;
    int pid = ++simulated_pid_counter;
    Logger::getInstance().info("RecoveryEngine", "[Simulation Mode] Spawned mock process '" + binary_path + "' with PID: " + std::to_string(pid));
    return pid;
#endif
}

// 2. Formats current wall-clock timestamp
std::string RecoveryManager::get_current_timestamp() const {
    auto now = std::chrono::system_clock::now();
    std::time_t time_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf{};
#if defined(_MSC_VER)
    localtime_s(&tm_buf, &time_now);
#elif defined(_WIN32)
    std::tm* tm_ptr = std::localtime(&time_now);
    if (tm_ptr) tm_buf = *tm_ptr;
#else
    localtime_r(&time_now, &tm_buf);
#endif
    std::ostringstream ss;
    ss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string RecoveryManager::status_to_string(ProcessStatus status) const {
    switch (status) {
        case ProcessStatus::STOPPED:          return "STOPPED";
        case ProcessStatus::RUNNING:          return "RUNNING";
        case ProcessStatus::CRASHED:          return "CRASHED";
        case ProcessStatus::RECOVERING:       return "RECOVERING";
        case ProcessStatus::FAILED_PERMANENT: return "FAILED_PERMANENT";
        default:                              return "UNKNOWN";
    }
}

std::string RecoveryManager::policy_to_string(RestartPolicy policy) const {
    switch (policy) {
        case RestartPolicy::IMMEDIATE: return "IMMEDIATE";
        case RestartPolicy::DELAYED:   return "DELAYED";
        default:                       return "UNKNOWN";
    }
}

// 3. Registers a critical process into supervision registry
bool RecoveryManager::register_process(const MonitoredProcess& process_config) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_monitored_processes.find(process_config.name) != m_monitored_processes.end()) {
        Logger::getInstance().warn("RecoveryEngine", "Process already registered: " + process_config.name);
        return false;
    }

    m_monitored_processes[process_config.name] = process_config;
    Logger::getInstance().info("RecoveryEngine", "Registered process '" + process_config.name + 
        "' [Policy: " + policy_to_string(process_config.policy) + 
        ", Max Retries: " + std::to_string(process_config.max_retries) + "]");
    return true;
}

// 4. Unregisters a process
bool RecoveryManager::unregister_process(const std::string& name) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_monitored_processes.find(name);
    if (it == m_monitored_processes.end()) {
        return false;
    }
    stop_process(name);
    m_monitored_processes.erase(it);
    Logger::getInstance().info("RecoveryEngine", "Unregistered process: " + name);
    return true;
}

// 5. Starts process execution and sets RUNNING state
bool RecoveryManager::start_process(const std::string& name) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_monitored_processes.find(name);
    if (it == m_monitored_processes.end()) {
        Logger::getInstance().error("RecoveryEngine", "Cannot start unregistered process: " + name);
        return false;
    }

    MonitoredProcess& proc = it->second;
    int pid = spawn_process(proc.binary_path, proc.args);
    if (pid > 0) {
        proc.pid = pid;
        proc.status = ProcessStatus::RUNNING;
        proc.retry_count = 0;
        proc.last_error_reason = "";
        Logger::getInstance().info("RecoveryEngine", "Successfully launched process '" + proc.name + "' with PID: " + std::to_string(pid));
        return true;
    } else {
        proc.status = ProcessStatus::CRASHED;
        proc.last_error_reason = "Spawn failed";
        Logger::getInstance().error("RecoveryEngine", "Failed to launch process: " + proc.name);
        return false;
    }
}

// 6. Manually stops monitored process
bool RecoveryManager::stop_process(const std::string& name) {
    auto it = m_monitored_processes.find(name);
    if (it == m_monitored_processes.end()) {
        return false;
    }

    MonitoredProcess& proc = it->second;
    if (proc.pid > 0) {
        m_process_manager.terminate_process(proc.pid, 15);
        proc.pid = -1;
    }
    proc.status = ProcessStatus::STOPPED;
    Logger::getInstance().info("RecoveryEngine", "Stopped monitored process: " + name);
    return true;
}

// 7. Evaluates health of a single process and performs self-healing recovery if failed
bool RecoveryManager::check_and_heal_process(MonitoredProcess& proc) {
    if (proc.status == ProcessStatus::STOPPED || proc.status == ProcessStatus::FAILED_PERMANENT) {
        return true; // No monitoring action for explicitly stopped or permanently failed processes
    }

    bool is_alive = false;
    bool is_zombie_state = false;
    std::string failure_detail = "";

#if defined(__linux__)
    if (proc.pid > 0) {
        int wstatus = 0;
        pid_t result = waitpid(proc.pid, &wstatus, WNOHANG);

        if (result == proc.pid) {
            // Child process exited or was terminated
            is_alive = false;
            if (WIFEXITED(wstatus)) {
                failure_detail = "Exited with code " + std::to_string(WEXITSTATUS(wstatus));
            } else if (WIFSIGNALED(wstatus)) {
                failure_detail = "Terminated by signal " + std::to_string(WTERMSIG(wstatus));
            }
        } else if (result == 0) {
            // Process exists and status hasn't changed
            is_alive = m_process_manager.is_process_running(proc.pid);
            if (is_alive && m_process_manager.is_zombie(proc.pid)) {
                is_zombie_state = true;
                failure_detail = "Zombie process state detected ('Z')";
            }
        } else {
            // Error or process does not exist
            is_alive = m_process_manager.is_process_running(proc.pid);
        }
    }
#else
    // Simulation logic for non-Linux hosts
    if (proc.binary_path.find("invalid") != std::string::npos || proc.binary_path.find("non_existent") != std::string::npos) {
        is_alive = false;
        failure_detail = "Binary execution error: Invalid binary path";
    } else {
        is_alive = (proc.pid > 0 && proc.status == ProcessStatus::RUNNING);
    }
#endif

    // If healthy and not zombie, return true
    if (is_alive && !is_zombie_state) {
        return true;
    }

    // --- FAILURE DETECTED ---
    proc.status = ProcessStatus::CRASHED;
    proc.last_error_reason = failure_detail.empty() ? "Unexpected termination / crash" : failure_detail;

    // 1. Log failure detection
    Logger::getInstance().error("RecoveryEngine", 
        "Process Crash Detected! Name: '" + proc.name + "' | PID: " + std::to_string(proc.pid) + " | Reason: " + proc.last_error_reason);

    // 2. Alert Integration: Generate WARNING Alert
    Logger::getInstance().warn("AlertManager", 
        "WARNING: Process failure detected for '" + proc.name + "' (PID: " + std::to_string(proc.pid) + "). Self-healing triggered.");

    // 3. Evaluate Recovery Policies (Max Retry Count & Cooldown)
    if (proc.retry_count < proc.max_retries) {
        proc.retry_count++;
        proc.status = ProcessStatus::RECOVERING;

        // Handle DELAYED policy cooldown interval
        if (proc.policy == RestartPolicy::DELAYED && proc.cooldown_seconds > 0) {
            Logger::getInstance().info("RecoveryEngine", 
                "Applying recovery policy DELAYED: Waiting " + std::to_string(proc.cooldown_seconds) + "s cooldown for '" + proc.name + "'...");
            std::this_thread::sleep_for(std::chrono::seconds(proc.cooldown_seconds));
        }

        // Log recovery attempt
        Logger::getInstance().info("RecoveryEngine", 
            "Self-Healing Attempt " + std::to_string(proc.retry_count) + "/" + std::to_string(proc.max_retries) + 
            " for process '" + proc.name + "'...");

        // Respawn failed process
        int new_pid = spawn_process(proc.binary_path, proc.args);
        if (new_pid > 0) {
            proc.pid = new_pid;
            proc.status = ProcessStatus::RUNNING;
            proc.last_recovery_time = get_current_timestamp();
            
            // Log successful recovery
            Logger::getInstance().info("RecoveryEngine", 
                "SUCCESS: Process '" + proc.name + "' automatically restored! New PID: " + std::to_string(new_pid));

            // Alert Integration: Recovery Success
            Logger::getInstance().info("AlertManager", 
                "RECOVERY SUCCESS: Process '" + proc.name + "' restored to healthy RUNNING state (PID: " + std::to_string(new_pid) + ")");
            return true;
        } else {
            // Spawn failed
            Logger::getInstance().error("RecoveryEngine", "Recovery attempt failed: Unable to spawn '" + proc.name + "'");
        }
    }

    // 4. Exhausted Retries: Permanent Failure State
    if (proc.retry_count >= proc.max_retries) {
        proc.status = ProcessStatus::FAILED_PERMANENT;
        proc.pid = -1;

        // Log failed recovery
        Logger::getInstance().critical("RecoveryEngine", 
            "CRITICAL RECOVERY FAILURE: Process '" + proc.name + "' exceeded max retries (" + 
            std::to_string(proc.max_retries) + "/" + std::to_string(proc.max_retries) + "). Self-healing aborted.");

        // Alert Integration: Generate CRITICAL Alert
        Logger::getInstance().critical("AlertManager", 
            "CRITICAL ALERT: Process '" + proc.name + "' is permanently FAILED! Autonomous recovery abandoned. Manual action required.");
    }

    return false;
}

// 8. Performs single tick health check across all registered processes
void RecoveryManager::perform_health_check() {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& pair : m_monitored_processes) {
        check_and_heal_process(pair.second);
    }
}

// 9. Background monitoring event loop
void RecoveryManager::monitor_loop() {
    while (m_is_monitoring.load()) {
        perform_health_check();
        std::this_thread::sleep_for(m_check_interval);
    }
}

// 10. Starts background monitoring thread
void RecoveryManager::start_monitoring() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_is_monitoring.load()) return;

    m_is_monitoring.store(true);
    m_monitor_thread = std::thread(&RecoveryManager::monitor_loop, this);
    Logger::getInstance().info("RecoveryEngine", "Autonomous Self-Healing monitoring loop started.");
}

// 11. Stops background monitoring thread
void RecoveryManager::stop_monitoring() {
    if (!m_is_monitoring.load()) return;

    m_is_monitoring.store(false);
    if (m_monitor_thread.joinable()) {
        m_monitor_thread.join();
    }
    Logger::getInstance().info("RecoveryEngine", "Autonomous Self-Healing monitoring loop stopped.");
}

// 12. Displays Dashboard Output
void RecoveryManager::display_dashboard() const {
    std::lock_guard<std::mutex> lock(m_mutex);

    std::cout << "\n==========================================================================================" << std::endl;
    std::cout << "                 SENTINEL OS: SELF-HEALING RECOVERY DASHBOARD                            " << std::endl;
    std::cout << "==========================================================================================" << std::endl;
    std::cout << " " << std::left << std::setw(18) << "PROCESS NAME"
              << " | " << std::setw(7)  << "PID"
              << " | " << std::setw(16) << "STATUS"
              << " | " << std::setw(14) << "RECOVERY COUNT"
              << " | " << std::setw(10) << "POLICY"
              << " | " << "LAST RECOVERY TIME" << std::endl;
    std::cout << "-------------------+---------+------------------+---------------+------------+--------------------" << std::endl;

    if (m_monitored_processes.empty()) {
        std::cout << " (No processes registered under self-healing engine supervision)" << std::endl;
    } else {
        for (const auto& pair : m_monitored_processes) {
            const MonitoredProcess& proc = pair.second;
            std::string pid_str = (proc.pid > 0) ? std::to_string(proc.pid) : "N/A";
            std::string recovery_str = std::to_string(proc.retry_count) + " / " + std::to_string(proc.max_retries);

            std::cout << " " << std::left << std::setw(18) << proc.name
                      << " | " << std::setw(7)  << pid_str
                      << " | " << std::setw(16) << status_to_string(proc.status)
                      << " | " << std::setw(14) << recovery_str
                      << " | " << std::setw(10) << policy_to_string(proc.policy)
                      << " | " << proc.last_recovery_time << std::endl;
        }
    }
    std::cout << "==========================================================================================" << std::endl;
}

// 13. Returns copy of all monitored process records
std::vector<MonitoredProcess> RecoveryManager::get_monitored_processes() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<MonitoredProcess> list;
    for (const auto& pair : m_monitored_processes) {
        list.push_back(pair.second);
    }
    return list;
}

// 14. Resets retry count for manual operator reset
bool RecoveryManager::reset_process_retry_count(const std::string& name) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_monitored_processes.find(name);
    if (it == m_monitored_processes.end()) return false;

    it->second.retry_count = 0;
    it->second.status = ProcessStatus::STOPPED;
    it->second.last_error_reason = "";
    Logger::getInstance().info("RecoveryEngine", "Operator reset retry count for process: " + name);
    return true;
}

// 15. Forcefully terminates or simulates a crash for testing self-healing
bool RecoveryManager::force_crash_process(const std::string& name) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_monitored_processes.find(name);
    if (it == m_monitored_processes.end()) return false;

    MonitoredProcess& proc = it->second;
    if (proc.pid > 0) {
        m_process_manager.terminate_process(proc.pid, 9);
    }
    proc.status = ProcessStatus::CRASHED;
    proc.last_error_reason = "Simulated crash signal (SIGKILL)";
    Logger::getInstance().warn("RecoveryEngine", "Force crash simulated for process: " + name);
    return true;
}
