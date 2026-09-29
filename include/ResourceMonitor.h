#ifndef SENTINEL_RESOURCE_MONITOR_H
#define SENTINEL_RESOURCE_MONITOR_H

#include <string>
#include <vector>
#include <cstdint>
#include <mutex>
#include "Common.h"

namespace Sentinel {

    /**
     * @struct CpuTicks
     * @brief Raw CPU tick counters read from /proc/stat
     */
    struct CpuTicks {
        uint64_t user{0};
        uint64_t nice{0};
        uint64_t system{0};
        uint64_t idle{0};
        uint64_t iowait{0};
        uint64_t irq{0};
        uint64_t softirq{0};
        uint64_t steal{0};

        uint64_t get_idle_ticks() const { return idle + iowait; }
        uint64_t get_total_ticks() const {
            return user + nice + system + idle + iowait + irq + softirq + steal;
        }
    };

    /**
     * @struct CpuStats
     * @brief Calculated CPU usage percentages
     */
    struct CpuStats {
        double overall_usage_percent{0.0};
        std::vector<double> per_core_usage;
    };

    /**
     * @struct MemoryStats
     * @brief Memory allocation details in Megabytes from /proc/meminfo
     */
    struct MemoryStats {
        double total_ram_mb{0.0};
        double free_ram_mb{0.0};
        double available_ram_mb{0.0};
        double used_ram_mb{0.0};
        double ram_usage_percent{0.0};
    };

    /**
     * @struct DiskStats
     * @brief Filesystem storage capacity details in Gigabytes from statvfs()
     */
    struct DiskStats {
        double total_disk_gb{0.0};
        double free_disk_gb{0.0};
        double used_disk_gb{0.0};
        double disk_usage_percent{0.0};
    };

    /**
     * @class ResourceMonitor
     * @brief Engine responsible for sampling Linux CPU, Memory, and Disk hardware metrics.
     */
    class ResourceMonitor {
    private:
        CpuTicks m_prev_cpu_ticks;
        CpuStats m_current_cpu_stats;
        MemoryStats m_current_mem_stats;
        DiskStats m_current_disk_stats;
        mutable std::mutex m_mutex;

        CpuTicks read_proc_stat_ticks();

    public:
        ResourceMonitor();
        ~ResourceMonitor() = default;

        /**
         * @brief Samples all system resources and updates internal metric state.
         */
        void update_metrics();

        /**
         * @brief Computes CPU utilization percentage using /proc/stat delta ticks.
         * @return Calculated CPU usage percentage (0.0% to 100.0%).
         */
        double calculate_cpu_usage();

        /**
         * @brief Parses /proc/meminfo to retrieve RAM metrics.
         * @return MemoryStats struct populated with RAM values.
         */
        MemoryStats get_memory_stats();

        /**
         * @brief Inspects filesystem utilization via statvfs().
         * @param mount_point Path to mount point (default "/").
         * @return DiskStats struct populated with storage values.
         */
        DiskStats get_disk_stats(const std::string& mount_point = "/");
    };

} // namespace Sentinel

#endif // SENTINEL_RESOURCE_MONITOR_H
