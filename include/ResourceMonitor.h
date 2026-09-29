#ifndef RESOURCE_MONITOR_H
#define RESOURCE_MONITOR_H

#include <string>
#include <cstdint>

// Struct to store CPU tick counters read from /proc/stat
struct CpuTicks {
    uint64_t idle_ticks{0};
    uint64_t total_ticks{0};
};

// Struct to hold calculated Memory metrics
struct MemoryInfo {
    double total_ram_mb{0.0};
    double free_ram_mb{0.0};
    double used_ram_mb{0.0};
    double ram_usage_percent{0.0};
};

// Struct to hold calculated Disk storage metrics
struct DiskInfo {
    double total_disk_gb{0.0};
    double free_disk_gb{0.0};
    double used_disk_gb{0.0};
    double disk_usage_percent{0.0};
};

/**
 * @class ResourceMonitor
 * @brief Handles real-time monitoring of CPU, RAM, and Disk resource utilization on Linux systems.
 */
class ResourceMonitor {
private:
    // Stores CPU tick values from previous sampling calculation
    CpuTicks m_prev_cpu_ticks;

    // Helper function to read raw ticks from /proc/stat
    CpuTicks read_proc_stat_ticks();

public:
    // Constructor initializes baseline CPU ticks
    ResourceMonitor();

    // Destructor
    ~ResourceMonitor() = default;

    // Computes CPU utilization percentage since last call
    double get_cpu_usage_percent();

    // Reads /proc/meminfo and calculates RAM usage
    MemoryInfo get_memory_info();

    // Inspects filesystem capacity using statvfs()
    DiskInfo get_disk_info(const std::string& mount_point = "/");
};

#endif // RESOURCE_MONITOR_H
