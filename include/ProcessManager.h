#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include <string>
#include <vector>

struct ProcessDetails {
    int pid{0};
    std::string name;
    char state{'R'};
    int ppid{0};
    uint64_t memory_kb{0};
};

class ProcessManager {
public:
    ProcessManager() = default;
    ~ProcessManager() = default;

    std::vector<int> get_all_pids();
    ProcessDetails get_process_details(int pid);
    std::vector<ProcessDetails> get_running_processes_list();
    bool send_signal_to_process(int pid, int signal_number);
};

#endif // PROCESS_MANAGER_H
