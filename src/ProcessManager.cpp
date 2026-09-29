#include "ProcessManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

#if defined(__linux__)
#include <sys/types.h>
#include <signal.h>
#endif

namespace fs = std::filesystem;

std::vector<int> ProcessManager::get_all_pids() {
    std::vector<int> pids;
    if (!fs::exists("/proc")) {
        return pids;
    }

    for (const auto& entry : fs::directory_iterator("/proc")) {
        if (entry.is_directory()) {
            std::string filename = entry.path().filename().string();
            if (std::all_of(filename.begin(), filename.end(), ::isdigit)) {
                pids.push_back(std::stoi(filename));
            }
        }
    }
    return pids;
}

ProcessDetails ProcessManager::get_process_details(int pid) {
    ProcessDetails details;
    details.pid = pid;
    details.name = "Unknown";
    details.state = 'U';

    std::string stat_path = "/proc/" + std::to_string(pid) + "/stat";
    std::ifstream file(stat_path);
    if (!file.is_open()) {
        return details;
    }

    std::string line;
    if (std::getline(file, line)) {
        size_t open_paren = line.find('(');
        size_t close_paren = line.rfind(')');
        if (open_paren != std::string::npos && close_paren != std::string::npos) {
            details.name = line.substr(open_paren + 1, close_paren - open_paren - 1);
            
            std::string remaining = line.substr(close_paren + 2);
            std::istringstream ss(remaining);
            ss >> details.state >> details.ppid;
        }
    }
    return details;
}

std::vector<ProcessDetails> ProcessManager::get_running_processes_list() {
    std::vector<ProcessDetails> list;
    std::vector<int> pids = get_all_pids();
    for (int pid : pids) {
        ProcessDetails d = get_process_details(pid);
        if (d.pid > 0) {
            list.push_back(d);
        }
    }
    return list;
}

bool ProcessManager::send_signal_to_process(int pid, int signal_number) {
#if defined(__linux__)
    return (kill(pid, signal_number) == 0);
#else
    (void)pid;
    (void)signal_number;
    return true;
#endif
}
