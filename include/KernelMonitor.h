#ifndef KERNEL_MONITOR_H
#define KERNEL_MONITOR_H

#include <string>
#include <vector>
#include <cstdint>

// Struct holding Kernel Version & Distro Information
struct KernelVersionInfo {
    std::string full_kernel_version;
    std::string kernel_release;
    std::string os_distro;
};

// Struct holding CPU Hardware Specifications
struct CpuHardwareInfo {
    std::string model_name;
    int num_cores{0};
    std::string architecture;
};

// Struct holding System Host & Uptime Details
struct SystemInfo {
    std::string hostname;
    double uptime_seconds{0.0};
    std::string os_details;
};

// Struct holding RAM & Swap Allocation Details
struct MemorySwapInfo {
    double total_ram_mb{0.0};
    double available_ram_mb{0.0};
    double total_swap_mb{0.0};
    double free_swap_mb{0.0};
};

/**
 * @class KernelMonitor
 * @brief Collects and formats Linux Kernel, CPU hardware, system uptime, and swap memory details.
 */
class KernelMonitor {
private:
    // Helper function to safely read the first line of a text file
    std::string read_first_line(const std::string& filepath);

public:
    KernelMonitor() = default;
    ~KernelMonitor() = default;

    // Reads /proc/version and uname for kernel release and OS details
    KernelVersionInfo get_kernel_version_info();

    // Reads /proc/cpuinfo for model name, core count, and architecture
    CpuHardwareInfo get_cpu_info();

    // Reads /proc/uptime and hostname system info
    SystemInfo get_system_info();

    // Reads /proc/meminfo for RAM and Swap metrics
    MemorySwapInfo get_memory_swap_info();

    // Helper method to display formatted summary of all kernel information
    void display_kernel_summary();
};

#endif // KERNEL_MONITOR_H
