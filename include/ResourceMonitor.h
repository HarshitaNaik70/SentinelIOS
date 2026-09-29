#ifndef RESOURCE_MONITOR_H
#define RESOURCE_MONITOR_H

#include <cstdint>
#include <string>

struct CpuTicks {
    uint64_t idle_ticks{0};
    uint64_t total_ticks{0};
};

struct MemoryInfo {
    double total_ram_mb{0.0};
    double free_ram_mb{0.0};
    double used_ram_mb{0.0};
    double ram_usage_percent{0.0};
};

struct DiskInfo {
    double total_disk_gb{0.0};
    double free_disk_gb{0.0};
    double used_disk_gb{0.0};
    double disk_usage_percent{0.0};
};

class ResourceMonitor {
private:
    CpuTicks m_prev_cpu_ticks;
    CpuTicks read_proc_stat_ticks();

public:
    ResourceMonitor();
    ~ResourceMonitor() = default;

    double get_cpu_usage_percent();
    MemoryInfo get_memory_info();
    DiskInfo get_disk_info(const std::string& mount_point = "/");
};

#endif // RESOURCE_MONITOR_H
