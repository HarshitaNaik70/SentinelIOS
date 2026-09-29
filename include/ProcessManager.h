#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include <string>
#include <vector>
#include <cstdint>

// Struct representing detailed metadata for a single Linux process
struct ProcessInfo {
    int pid{0};              // Process ID
    int ppid{0};             // Parent Process ID
    std::string name;        // Binary / Executable Name
    char state{'U'};         // Process State: 'R' (Running), 'S' (Sleeping), 'Z' (Zombie), 'D' (Disk Sleep), 'T' (Stopped)
    uint64_t memory_kb{0};   // Resident Set Size RAM Memory Usage in Kilobytes (kB)
};

// Struct aggregating process state counts across the system
struct ProcessStatsSummary {
    int total_processes{0};
    int running_count{0};
    int sleeping_count{0};
    int zombie_count{0};
    int stopped_count{0};
    int other_count{0};
};

/**
 * @class ProcessManager
 * @brief Discovers, monitors, searches, and statistics-aggregates Linux processes using /proc.
 */
class ProcessManager {
private:
    // Helper function to scan numerical PID subdirectories in /proc
    std::vector<int> scan_proc_pids();

public:
    ProcessManager() = default;
    ~ProcessManager() = default;

    // Discovers and parses details for all active processes
    std::vector<ProcessInfo> get_all_processes();

    // Retrieves metadata for a specific PID
    ProcessInfo get_process_by_pid(int pid);

    // Searches for processes matching a string name query
    std::vector<ProcessInfo> search_processes_by_name(const std::string& name_query);

    // Calculates process state statistics summary (Running, Sleeping, Zombie counts)
    ProcessStatsSummary get_process_stats_summary();

    // Renders formatted top process list to console
    void display_top_processes(int limit = 10);

    // Displays aggregate process state summary statistics
    void display_process_summary();

    // Checks if a process with given PID is currently active
    bool is_process_running(int pid);

    // Checks if a process with given PID is in Zombie state ('Z')
    bool is_zombie(int pid);

    // Sends a POSIX signal to terminate a process (default SIGTERM = 15, SIGKILL = 9)
    bool terminate_process(int pid, int signal_num = 15);
};

#endif // PROCESS_MANAGER_H
