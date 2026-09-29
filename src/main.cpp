#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
#include "ResourceMonitor.h"
#include "KernelMonitor.h"

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "  SentinelOS: Milestone 3 - Kernel & System Info Monitor  " << std::endl;
    std::cout << "==========================================================" << std::endl;

    // Instantiate Milestone 3 KernelMonitor
    KernelMonitor kernel_monitor;
    kernel_monitor.display_kernel_summary();

    // Instantiate Milestone 2 ResourceMonitor
    ResourceMonitor resource_monitor;

    std::cout << "\n[Info] Starting real-time system resource sampling..." << std::endl;
    std::cout << "----------------------------------------------------------" << std::endl;

    // Execute 3 sampling iterations at 1-second intervals
    for (int sample = 1; sample <= 3; ++sample) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        double cpu_usage = resource_monitor.get_cpu_usage_percent();
        MemoryInfo mem = resource_monitor.get_memory_info();
        DiskInfo disk = resource_monitor.get_disk_info("/");

        std::cout << "Sample #" << sample << " | "
                  << "CPU: " << std::fixed << std::setprecision(2) << cpu_usage << "% | "
                  << "RAM: " << std::setprecision(1) << mem.used_ram_mb << " MB / " << mem.total_ram_mb << " MB (" << mem.ram_usage_percent << "%) | "
                  << "Disk: " << disk.used_disk_gb << " GB / " << disk.total_disk_gb << " GB (" << disk.disk_usage_percent << "%)"
                  << std::endl;
    }

    std::cout << "----------------------------------------------------------" << std::endl;
    std::cout << "[Success] Milestone 3 Integrated Test Completed Successfully." << std::endl;
    std::cout << "==========================================================" << std::endl;

    return 0;
}
