#ifndef KERNEL_MONITOR_H
#define KERNEL_MONITOR_H

#include <string>

struct SystemKernelInfo {
    std::string kernel_version;
    std::string os_name;
    double uptime_seconds{0.0};
    double load_avg_1min{0.0};
    double load_avg_5min{0.0};
    double load_avg_15min{0.0};
    int total_processes{0};
};

class KernelMonitor {
public:
    KernelMonitor() = default;
    ~KernelMonitor() = default;

    SystemKernelInfo get_system_kernel_info();
    std::string get_kernel_version_string();
};

#endif // KERNEL_MONITOR_H
