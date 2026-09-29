#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
#include "ResourceMonitor.h"
#include "KernelMonitor.h"
#include "ProcessManager.h"

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "  SentinelOS: Milestone 4 - Process Monitoring Module     " << std::endl;
    std::cout << "==========================================================" << std::endl;

    // 1. Milestone 3: Kernel Summary
    KernelMonitor kernel_monitor;
    kernel_monitor.display_kernel_summary();

    // 2. Milestone 4: Process Monitoring Summary & Top Processes
    ProcessManager process_manager;
    process_manager.display_process_summary();
    process_manager.display_top_processes(8);

    // 3. Process Search Demo
    std::cout << "\n[Process Search Demo] Searching for 'systemd' / 'init' processes:" << std::endl;
    std::vector<ProcessInfo> search_results = process_manager.search_processes_by_name("systemd");
    if (search_results.empty()) {
        search_results = process_manager.search_processes_by_name("init");
    }
    for (const auto& proc : search_results) {
        std::cout << "  Found PID: " << proc.pid << " | Name: " << proc.name << " | State: " << proc.state << std::endl;
    }

    // 4. Milestone 2: Real-time Resource Sampling Ticks
    ResourceMonitor resource_monitor;
    std::cout << "\n[Info] Starting real-time system resource sampling..." << std::endl;
    std::cout << "----------------------------------------------------------" << std::endl;

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
    std::cout << "[Success] Milestone 4 Process Monitoring Integrated Test Completed." << std::endl;
    std::cout << "==========================================================" << std::endl;

    return 0;
}
