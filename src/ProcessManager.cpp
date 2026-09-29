#include "ProcessManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <iomanip>

namespace fs = std::filesystem;

// 1. Scans /proc directory for numerical subdirectories representing active PIDs
std::vector<int> ProcessManager::scan_proc_pids() {
    std::vector<int> pids;
    if (!fs::exists("/proc")) {
        return pids;
    }

    for (const auto& entry : fs::directory_iterator("/proc")) {
        if (entry.is_directory()) {
            std::string filename = entry.path().filename().string();
            // Check if directory name is entirely numeric
            if (std::all_of(filename.begin(), filename.end(), ::isdigit)) {
                pids.push_back(std::stoi(filename));
            }
        }
    }
    std::sort(pids.begin(), pids.end());
    return pids;
}

// 2. Extracts process details for a specific PID from /proc/[pid]/stat & /proc/[pid]/status
ProcessInfo ProcessManager::get_process_by_pid(int pid) {
    ProcessInfo info;
    info.pid = pid;
    info.name = "Unknown";
    info.state = 'U';
    info.ppid = 0;
    info.memory_kb = 0;

    // Parse /proc/[pid]/stat
    std::string stat_path = "/proc/" + std::to_string(pid) + "/stat";
    std::ifstream stat_file(stat_path);
    if (stat_file.is_open()) {
        std::string line;
        if (std::getline(stat_file, line)) {
            // Extract process name enclosed in parentheses ()
            size_t open_paren = line.find('(');
            size_t close_paren = line.rfind(')');
            if (open_paren != std::string::npos && close_paren != std::string::npos && close_paren > open_paren) {
                info.name = line.substr(open_paren + 1, close_paren - open_paren - 1);

                // Parse state and PPID from remainder of string
                std::string remaining = line.substr(close_paren + 2);
                std::istringstream ss(remaining);
                ss >> info.state >> info.ppid;
            }
        }
    }

    // Parse /proc/[pid]/status for VmRSS memory usage
    std::string status_path = "/proc/" + std::to_string(pid) + "/status";
    std::ifstream status_file(status_path);
    if (status_file.is_open()) {
        std::string key;
        uint64_t val;
        std::string unit;
        while (status_file >> key >> val >> unit) {
            if (key == "VmRSS:") {
                info.memory_kb = val;
                break;
            }
        }
    }

    return info;
}

// 3. Discovers all active processes across the system
std::vector<ProcessInfo> ProcessManager::get_all_processes() {
    std::vector<ProcessInfo> process_list;
    std::vector<int> pids = scan_proc_pids();

    for (int pid : pids) {
        ProcessInfo info = get_process_by_pid(pid);
        if (info.pid > 0) {
            process_list.push_back(info);
        }
    }
    return process_list;
}

// 4. Searches processes matching a target string query
std::vector<ProcessInfo> ProcessManager::search_processes_by_name(const std::string& name_query) {
    std::vector<ProcessInfo> results;
    std::vector<ProcessInfo> all = get_all_processes();

    std::string query_lower = name_query;
    std::transform(query_lower.begin(), query_lower.end(), query_lower.begin(), ::tolower);

    for (const auto& proc : all) {
        std::string proc_name_lower = proc.name;
        std::transform(proc_name_lower.begin(), proc_name_lower.end(), proc_name_lower.begin(), ::tolower);

        if (proc_name_lower.find(query_lower) != std::string::npos) {
            results.push_back(proc);
        }
    }
    return results;
}

// 5. Calculates summary counts for process states
ProcessStatsSummary ProcessManager::get_process_stats_summary() {
    ProcessStatsSummary summary{};
    std::vector<ProcessInfo> all = get_all_processes();
    summary.total_processes = static_cast<int>(all.size());

    for (const auto& proc : all) {
        switch (proc.state) {
            case 'R': summary.running_count++; break;
            case 'S': summary.sleeping_count++; break;
            case 'Z': summary.zombie_count++; break;
            case 'T': summary.stopped_count++; break;
            default:  summary.other_count++; break;
        }
    }
    return summary;
}

// 6. Displays top processes list
void ProcessManager::display_top_processes(int limit) {
    std::vector<ProcessInfo> all = get_all_processes();
    std::cout << "\n----------------------------------------------------------" << std::endl;
    std::cout << "               PROCESS MONITORING TABLE                   " << std::endl;
    std::cout << "----------------------------------------------------------" << std::endl;
    std::cout << " " << std::setw(7) << "PID" 
              << " | " << std::setw(6) << "PPID" 
              << " | " << std::setw(5) << "STATE" 
              << " | " << std::setw(10) << "RAM (KB)" 
              << " | " << "PROCESS NAME" << std::endl;
    std::cout << "-------+--------+-------+------------+--------------------" << std::endl;

    int count = 0;
    for (const auto& proc : all) {
        std::cout << " " << std::setw(7) << proc.pid 
                  << " | " << std::setw(6) << proc.ppid 
                  << " | " << std::setw(5) << proc.state 
                  << " | " << std::setw(10) << proc.memory_kb 
                  << " | " << proc.name << std::endl;
        if (++count >= limit) break;
    }
    std::cout << "----------------------------------------------------------" << std::endl;
}

// 7. Displays process statistics summary
void ProcessManager::display_process_summary() {
    ProcessStatsSummary summary = get_process_stats_summary();
    std::cout << "\n[Process Summary] Total: " << summary.total_processes
              << " | Running (R): " << summary.running_count
              << " | Sleeping (S): " << summary.sleeping_count
              << " | Zombie (Z): " << summary.zombie_count
              << " | Stopped (T): " << summary.stopped_count
              << std::endl;
}
