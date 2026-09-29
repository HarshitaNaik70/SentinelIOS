#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
#include "ResourceMonitor.h"

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "  SentinelOS: Milestone 2 - Resource Monitoring Module    " << std::endl;
    std::cout << "==========================================================" << std::endl;

    // Instantiate ResourceMonitor object
    ResourceMonitor monitor;

    std::cout << "[Info] Starting real-time system resource sampling..." << std::endl;
    std::cout << "----------------------------------------------------------" << std::endl;

    // Execute 5 sampling iterations at 1-second intervals
    for (int sample = 1; sample <= 5; ++sample) {
        // Sleep for 1000 milliseconds to allow CPU tick delta accumulation
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        // Retrieve CPU usage percentage
        double cpu_usage = monitor.get_cpu_usage_percent();

        // Retrieve Memory information
        MemoryInfo mem = monitor.get_memory_info();

        // Retrieve Disk information for root "/" mount point
        DiskInfo disk = monitor.get_disk_info("/");

        // Display results cleanly
        std::cout << "Sample #" << sample << " | "
                  << "CPU: " << std::fixed << std::setprecision(2) << cpu_usage << "% | "
                  << "RAM: " << std::setprecision(1) << mem.used_ram_mb << " MB / " << mem.total_ram_mb << " MB (" << mem.ram_usage_percent << "%) | "
                  << "Disk: " << disk.used_disk_gb << " GB / " << disk.total_disk_gb << " GB (" << disk.disk_usage_percent << "%)"
                  << std::endl;
    }

    std::cout << "----------------------------------------------------------" << std::endl;
    std::cout << "[Success] Milestone 2 Resource Monitoring Test Completed." << std::endl;
    std::cout << "==========================================================" << std::endl;

    return 0;
}
