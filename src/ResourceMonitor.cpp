#include "ResourceMonitor.h"
#include <fstream>
#include <sstream>
#include <iostream>

#if defined(__linux__)
#include <sys/statvfs.h>
#endif

// Constructor initializes baseline CPU ticks upon object creation
ResourceMonitor::ResourceMonitor() {
    m_prev_cpu_ticks = read_proc_stat_ticks();
}

// Helper Function: Reads the first line of /proc/stat to extract CPU ticks
CpuTicks ResourceMonitor::read_proc_stat_ticks() {
    CpuTicks ticks{0, 0};

    // Open /proc/stat file provided by the Linux kernel
    std::ifstream file("/proc/stat");
    if (!file.is_open()) {
        return ticks;
    }

    std::string line;
    // Read the first line which starts with "cpu  ..."
    if (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string cpu_label;
        uint64_t user, nice, system, idle, iowait, irq, softirq, steal;

        // Parse individual tick counters
        ss >> cpu_label >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

        // Idle ticks include idle time and iowait time
        ticks.idle_ticks = idle + iowait;

        // Total ticks is the sum of all CPU state durations
        ticks.total_ticks = user + nice + system + idle + iowait + irq + softirq + steal;
    }

    return ticks;
}

// Calculates CPU usage percentage between consecutive calls
double ResourceMonitor::get_cpu_usage_percent() {
    CpuTicks curr_ticks = read_proc_stat_ticks();

    // Calculate time deltas since previous check
    uint64_t idle_delta = curr_ticks.idle_ticks - m_prev_cpu_ticks.idle_ticks;
    uint64_t total_delta = curr_ticks.total_ticks - m_prev_cpu_ticks.total_ticks;

    // Save current ticks as baseline for next call
    m_prev_cpu_ticks = curr_ticks;

    // Prevent division by zero if total_delta is 0
    if (total_delta == 0) {
        return 0.0;
    }

    // CPU Usage % = (1.0 - (idle_delta / total_delta)) * 100.0
    double usage = (1.0 - static_cast<double>(idle_delta) / static_cast<double>(total_delta)) * 100.0;

    // Clamp value between 0.0% and 100.0%
    if (usage < 0.0) usage = 0.0;
    if (usage > 100.0) usage = 100.0;

    return usage;
}

// Reads /proc/meminfo to retrieve RAM capacity and usage
MemoryInfo ResourceMonitor::get_memory_info() {
    MemoryInfo mem{0.0, 0.0, 0.0, 0.0};

    // Open /proc/meminfo file
    std::ifstream file("/proc/meminfo");
    if (!file.is_open()) {
        return mem;
    }

    std::string key;
    uint64_t value;
    std::string unit;

    uint64_t total_kb = 0;
    uint64_t available_kb = 0;

    // Parse key-value pairs line by line
    while (file >> key >> value >> unit) {
        if (key == "MemTotal:") {
            total_kb = value;
        } else if (key == "MemAvailable:") {
            available_kb = value;
        }
    }

    // Calculate RAM usage metrics in Megabytes (MB)
    if (total_kb > 0) {
        mem.total_ram_mb = static_cast<double>(total_kb) / 1024.0;
        mem.free_ram_mb = static_cast<double>(available_kb) / 1024.0;
        mem.used_ram_mb = mem.total_ram_mb - mem.free_ram_mb;
        mem.ram_usage_percent = (mem.used_ram_mb / mem.total_ram_mb) * 100.0;
    }

    return mem;
}

// Inspects storage filesystem space using POSIX statvfs()
DiskInfo ResourceMonitor::get_disk_info(const std::string& mount_point) {
    DiskInfo disk{0.0, 0.0, 0.0, 0.0};

#if defined(__linux__)
    struct statvfs stat;
    // Execute statvfs POSIX system call on target mount point
    if (statvfs(mount_point.c_str(), &stat) == 0) {
        uint64_t total_bytes = stat.f_blocks * stat.f_frsize;
        uint64_t free_bytes = stat.f_bavail * stat.f_frsize;
        uint64_t used_bytes = total_bytes - free_bytes;

        // Convert bytes to Gigabytes (GB)
        disk.total_disk_gb = static_cast<double>(total_bytes) / (1024.0 * 1024.0 * 1024.0);
        disk.free_disk_gb = static_cast<double>(free_bytes) / (1024.0 * 1024.0 * 1024.0);
        disk.used_disk_gb = static_cast<double>(used_bytes) / (1024.0 * 1024.0 * 1024.0);
        disk.disk_usage_percent = (disk.used_disk_gb / disk.total_disk_gb) * 100.0;
    }
#else
    // Simulation values for testing on non-Linux systems
    (void)mount_point;
    disk.total_disk_gb = 100.0;
    disk.used_disk_gb = 45.0;
    disk.free_disk_gb = 55.0;
    disk.disk_usage_percent = 45.0;
#endif

    return disk;
}
