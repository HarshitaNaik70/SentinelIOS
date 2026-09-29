#ifndef RECOVERY_MANAGER_H
#define RECOVERY_MANAGER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <atomic>
#include <chrono>
#include "ThreadCompat.h"

#include "ProcessManager.h"
#include "AlertManager.h"

/**
 * @enum RestartPolicy
 * @brief Configurable restart behavior policy for monitored processes.
 */
enum class RestartPolicy {
    IMMEDIATE,   // Instantly attempt process restart upon failure detection
    DELAYED      // Wait for a configured cooldown interval before attempting restart
};

/**
 * @enum ProcessStatus
 * @brief Lifecycle status of a supervised critical process.
 */
enum class ProcessStatus {
    STOPPED,            // Process is not running and monitoring is paused
    RUNNING,            // Process is healthy and executing normally
    CRASHED,            // Process failure, crash, or unexpected exit detected
    RECOVERING,         // Self-healing restart operation in progress
    FAILED_PERMANENT    // Maximum retry attempts exceeded; marked as permanently failed
};

/**
 * @struct MonitoredProcess
 * @brief Holds configuration and runtime telemetry for a supervised system process.
 */
struct MonitoredProcess {
    std::string name;                               // Unique human-readable identifier (e.g., "WorkerDaemon")
    std::string binary_path;                        // Executable path (e.g., "/bin/sleep", "/usr/bin/python3")
    std::vector<std::string> args;                  // Command line arguments passed to executable
    int pid{-1};                                    // Active PID (-1 if process is inactive)
    RestartPolicy policy{RestartPolicy::IMMEDIATE}; // Configured restart policy
    int max_retries{3};                             // Maximum allowed restart retries before giving up
    int retry_count{0};                             // Current consecutive restart retry counter
    int cooldown_seconds{2};                        // Cooldown delay in seconds for DELAYED policy
    ProcessStatus status{ProcessStatus::STOPPED};   // Current operational state
    std::string last_recovery_time{"N/A"};          // Timestamp of the most recent recovery action
    std::string last_error_reason{""};              // Description of the last observed failure
};

/**
 * @class RecoveryManager
 * @brief Autonomous Self-Healing Recovery Engine for SentinelOS.
 *
 * Continuously monitors critical system processes for crashes, unexpected terminations, or zombie states.
 * Automatically restarts failed processes using POSIX fork/exec APIs, enforces configurable recovery policies,
 * logs recovery lifecycle events, dispatches WARNING/CRITICAL alerts via AlertManager, and renders a live dashboard.
 */
class RecoveryManager {
private:
    std::unordered_map<std::string, MonitoredProcess> m_monitored_processes;
    mutable std::mutex m_mutex;

    std::thread m_monitor_thread;
    std::atomic<bool> m_is_monitoring{false};
    std::chrono::milliseconds m_check_interval{1000};

    ProcessManager m_process_manager;
    AlertManager m_alert_manager;

    // Internal execution helpers
    void monitor_loop();
    bool check_and_heal_process(MonitoredProcess& proc);
    int spawn_process(const std::string& binary_path, const std::vector<std::string>& args);
    std::string get_current_timestamp() const;
    std::string status_to_string(ProcessStatus status) const;
    std::string policy_to_string(RestartPolicy policy) const;

public:
    RecoveryManager();
    ~RecoveryManager();

    // Prevent copying to maintain thread ownership and mutex safety
    RecoveryManager(const RecoveryManager&) = delete;
    RecoveryManager& operator=(const RecoveryManager&) = delete;

    // Registers a new critical process for autonomous supervision
    bool register_process(const MonitoredProcess& process_config);

    // Unregisters a process from supervision
    bool unregister_process(const std::string& name);

    // Starts supervising and spawns process if not already active
    bool start_process(const std::string& name);

    // Stops a monitored process manually and pauses supervision
    bool stop_process(const std::string& name);

    // Starts the background monitoring loop on a dedicated thread
    void start_monitoring();

    // Stops the background monitoring loop gracefully
    void stop_monitoring();

    // Triggers a single synchronous health check across all supervised processes
    void perform_health_check();

    // Displays the formatted Self-Healing Recovery Engine Dashboard
    void display_dashboard() const;

    // Returns a copy of all monitored process configurations and current state
    std::vector<MonitoredProcess> get_monitored_processes() const;

    // Resets retry counters for a process to clear a permanent failure state
    bool reset_process_retry_count(const std::string& name);

    // Forcefully terminates or simulates a crash for testing self-healing
    bool force_crash_process(const std::string& name);
};

#endif // RECOVERY_MANAGER_H
