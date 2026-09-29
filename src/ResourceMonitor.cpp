#include "ResourceMonitor.h"
#include <fstream>
#include <sstream>
#include <iostream>

#if defined(__linux__)
#include <sys/statvfs.h>
#endif

namespace Sentinel {

    ResourceMonitor::ResourceMonitor() {
        m_prev_cpu_ticks = read_proc_stat_ticks();
    }

    CpuTicks ResourceMonitor::read_proc_stat_ticks() {
        CpuTicks ticks{};
        std::ifstream file("/proc/stat");
        if (!file.is_open()) {
            return ticks;
        }

        std::string line;
        if (std::getline(file, line)) {
            std::istringstream ss(line);
            std::string cpu_label;
            ss >> cpu_label >> ticks.user >> ticks.nice >> ticks.system 
               >> ticks.idle >> ticks.iowait >> ticks.irq >> ticks.softirq >> ticks.steal;
        }
        return ticks;
    }

    double ResourceMonitor::calculate_cpu_usage() {
        std::lock_guard<std::mutex> lock(m_mutex);
        CpuTicks curr_ticks = read_proc_stat_ticks();

        uint64_t idle_delta = curr_ticks.get_idle_ticks() - m_prev_cpu_ticks.get_idle_ticks();
        uint64_t total_delta = curr_ticks.get_total_ticks() - m_prev_cpu_ticks.get_total_ticks();

        m_prev_cpu_ticks = curr_ticks;

        if (total_delta == 0) {
            return 0.0;
        }

        double usage = (1.0 - static_cast<double>(idle_delta) / static_cast<double>(total_delta)) * 100.0;
        if (usage < 0.0) usage = 0.0;
        if (usage > 100.0) usage = 100.0;

        m_current_cpu_stats.overall_usage_percent = usage;
        return usage;
    }

    MemoryStats ResourceMonitor::get_memory_stats() {
        std::lock_guard<std::mutex> lock(m_mutex);
        MemoryStats mem{};
        std::ifstream file("/proc/meminfo");
        if (!file.is_open()) {
            return mem;
        }

        std::string key;
        uint64_t value;
        std::string unit;

        uint64_t total_kb = 0;
        uint64_t available_kb = 0;
        uint64_t free_kb = 0;

        while (file >> key >> value >> unit) {
            if (key == "MemTotal:") {
                total_kb = value;
            } else if (key == "MemAvailable:") {
                available_kb = value;
            } else if (key == "MemFree:") {
                free_kb = value;
            }
        }

        if (total_kb > 0) {
            mem.total_ram_mb = static_cast<double>(total_kb) / 1024.0;
            mem.free_ram_mb = static_cast<double>(free_kb) / 1024.0;
            mem.available_ram_mb = static_cast<double>(available_kb) / 1024.0;
            mem.used_ram_mb = mem.total_ram_mb - mem.available_ram_mb;
            mem.ram_usage_percent = (mem.used_ram_mb / mem.total_ram_mb) * 100.0;
        }

        m_current_mem_stats = mem;
        return mem;
    }

    DiskStats ResourceMonitor::get_disk_stats(const std::string& mount_point) {
        std::lock_guard<std::mutex> lock(m_mutex);
        DiskStats disk{};
#if defined(__linux__)
        struct statvfs stat;
        if (statvfs(mount_point.c_str(), &stat) == 0) {
            uint64_t total_bytes = stat.f_blocks * stat.f_frsize;
            uint64_t free_bytes = stat.f_bavail * stat.f_frsize;
            uint64_t used_bytes = total_bytes - free_bytes;

            disk.total_disk_gb = static_cast<double>(total_bytes) / (1024.0 * 1024.0 * 1024.0);
            disk.free_disk_gb = static_cast<double>(free_bytes) / (1024.0 * 1024.0 * 1024.0);
            disk.used_disk_gb = static_cast<double>(used_bytes) / (1024.0 * 1024.0 * 1024.0);
            disk.disk_usage_percent = (disk.used_disk_gb / disk.total_disk_gb) * 100.0;
        }
#else
        disk.total_disk_gb = 100.0;
        disk.used_disk_gb = 45.0;
        disk.free_disk_gb = 55.0;
        disk.disk_usage_percent = 45.0;
#endif
        m_current_disk_stats = disk;
        return disk;
    }

    void ResourceMonitor::update_metrics() {
        calculate_cpu_usage();
        get_memory_stats();
        get_disk_stats("/");
    }

} // namespace Sentinel
