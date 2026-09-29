#include "ResourceMonitor.h"
#include <fstream>
#include <sstream>
#include <iostream>

#if defined(__linux__)
#include <sys/statvfs.h>
#endif

ResourceMonitor::ResourceMonitor() {
    m_prev_cpu_ticks = read_proc_stat_ticks();
}

CpuTicks ResourceMonitor::read_proc_stat_ticks() {
    CpuTicks ticks{0, 0};
    std::ifstream file("/proc/stat");
    if (!file.is_open()) {
        return ticks;
    }

    std::string line;
    if (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string cpu_label;
        uint64_t user, nice, system, idle, iowait, irq, softirq, steal;
        ss >> cpu_label >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

        ticks.idle_ticks = idle + iowait;
        ticks.total_ticks = user + nice + system + idle + iowait + irq + softirq + steal;
    }
    return ticks;
}

double ResourceMonitor::get_cpu_usage_percent() {
    CpuTicks curr_ticks = read_proc_stat_ticks();

    uint64_t idle_delta = curr_ticks.idle_ticks - m_prev_cpu_ticks.idle_ticks;
    uint64_t total_delta = curr_ticks.total_ticks - m_prev_cpu_ticks.total_ticks;

    m_prev_cpu_ticks = curr_ticks;

    if (total_delta == 0) {
        return 0.0;
    }

    double usage = (1.0 - (double)idle_delta / (double)total_delta) * 100.0;
    return (usage < 0.0) ? 0.0 : (usage > 100.0 ? 100.0 : usage);
}

MemoryInfo ResourceMonitor::get_memory_info() {
    MemoryInfo mem{0.0, 0.0, 0.0, 0.0};
    std::ifstream file("/proc/meminfo");
    if (!file.is_open()) {
        return mem;
    }

    std::string key;
    uint64_t value;
    std::string unit;

    uint64_t total_kb = 0;
    uint64_t available_kb = 0;

    while (file >> key >> value >> unit) {
        if (key == "MemTotal:") {
            total_kb = value;
        } else if (key == "MemAvailable:") {
            available_kb = value;
        }
    }

    if (total_kb > 0) {
        mem.total_ram_mb = (double)total_kb / 1024.0;
        mem.free_ram_mb = (double)available_kb / 1024.0;
        mem.used_ram_mb = mem.total_ram_mb - mem.free_ram_mb;
        mem.ram_usage_percent = (mem.used_ram_mb / mem.total_ram_mb) * 100.0;
    }

    return mem;
}

DiskInfo ResourceMonitor::get_disk_info(const std::string& mount_point) {
    DiskInfo disk{0.0, 0.0, 0.0, 0.0};
#if defined(__linux__)
    struct statvfs stat;
    if (statvfs(mount_point.c_str(), &stat) == 0) {
        uint64_t total_bytes = stat.f_blocks * stat.f_frsize;
        uint64_t free_bytes = stat.f_bavail * stat.f_frsize;
        uint64_t used_bytes = total_bytes - free_bytes;

        disk.total_disk_gb = (double)total_bytes / (1024.0 * 1024.0 * 1024.0);
        disk.free_disk_gb = (double)free_bytes / (1024.0 * 1024.0 * 1024.0);
        disk.used_disk_gb = (double)used_bytes / (1024.0 * 1024.0 * 1024.0);
        disk.disk_usage_percent = (disk.used_disk_gb / disk.total_disk_gb) * 100.0;
    }
#else
    // Fallback simulation for non-Linux host testing
    disk.total_disk_gb = 100.0;
    disk.used_disk_gb = 45.0;
    disk.free_disk_gb = 55.0;
    disk.disk_usage_percent = 45.0;
#endif
    return disk;
}
