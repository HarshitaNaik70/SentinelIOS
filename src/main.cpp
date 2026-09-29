#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>

#include "Logger.h"
#include "AlertManager.h"
#include "ResourceMonitor.h"
#include "KernelMonitor.h"
#include "ProcessManager.h"

int main() {
    // 1. Log Application Startup
    Logger::getInstance().info("Main", "Starting SentinelOS Daemon Platform (Milestone 5)...");

    std::cout << "==========================================================" << std::endl;
    std::cout << "  SentinelOS: Milestone 5 - Logging & Alert System        " << std::endl;
    std::cout << "==========================================================" << std::endl;

    // 2. Milestone 3: Kernel Summary & Logging
    KernelMonitor kernel_monitor;
    kernel_monitor.display_kernel_summary();
    Logger::getInstance().info("KernelMonitor", "Fetched system kernel and hardware specifications.");

    // 3. Milestone 4: Process Monitoring & Anomaly Alerting
    ProcessManager process_manager;
    process_manager.display_process_summary();
    process_manager.display_top_processes(5);

    std::vector<ProcessInfo> all_processes = process_manager.get_all_processes();
    Logger::getInstance().info("ProcessManager", "Discovered " + std::to_string(all_processes.size()) + " active processes.");

    // Instantiate Milestone 5 AlertManager
    AlertManager alert_manager(90.0, 85.0, 90.0);
    alert_manager.check_process_anomalies(all_processes);

    // 4. Milestone 2: Resource Sampling with Alert Evaluation
    ResourceMonitor resource_monitor;
    Logger::getInstance().info("ResourceMonitor", "Starting real-time system resource sampling ticks...");

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

        // Evaluate thresholds for alerts
        alert_manager.check_resource_thresholds(cpu_usage, mem.ram_usage_percent, disk.disk_usage_percent);
    }

    std::cout << "----------------------------------------------------------" << std::endl;

    // 5. Log Application Shutdown
    Logger::getInstance().info("Main", "SentinelOS Milestone 5 Execution Completed Successfully.");
    std::cout << "[Success] Milestone 5 Integrated Logging Test Completed." << std::endl;
    std::cout << "==========================================================" << std::endl;

    return 0;
}
