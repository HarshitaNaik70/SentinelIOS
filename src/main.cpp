#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>

#include "AlertManager.h"
#include "ResourceMonitor.h"
#include "ProcessManager.h"
#include "KernelMonitor.h"

int main() {
    AlertManager logger("sentinel_stage4.log");
    logger.info("Main", "Starting SentinelOS Stage 4 Prototype Daemon...");

    ResourceMonitor res_mon;
    ProcessManager proc_mgr;
    KernelMonitor kern_mon;

    std::cout << "==========================================================" << std::endl;
    std::cout << "   SentinelOS: System Monitoring Prototype (Stage 4)      " << std::endl;
    std::cout << "==========================================================" << std::endl;

    // Display Kernel Info
    SystemKernelInfo kinfo = kern_mon.get_system_kernel_info();
    std::cout << "[Kernel Info] " << kinfo.kernel_version << std::endl;
    std::cout << "[Uptime] " << std::fixed << std::setprecision(1) << kinfo.uptime_seconds << " seconds | "
              << "Load Avg (1/5/15m): " << kinfo.load_avg_1min << ", " << kinfo.load_avg_5min << ", " << kinfo.load_avg_15min << std::endl;
    std::cout << "----------------------------------------------------------" << std::endl;

    // Run 3 sampling ticks
    for (int tick = 1; tick <= 3; ++tick) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        double cpu_perc = res_mon.get_cpu_usage_percent();
        MemoryInfo mem = res_mon.get_memory_info();
        DiskInfo disk = res_mon.get_disk_info("/");

        std::cout << "\n--- Sample Tick #" << tick << " ---" << std::endl;
        std::cout << "CPU Usage  : " << std::fixed << std::setprecision(2) << cpu_perc << " %" << std::endl;
        std::cout << "RAM Usage  : " << mem.used_ram_mb << " MB / " << mem.total_ram_mb << " MB (" << mem.ram_usage_percent << " %)" << std::endl;
        std::cout << "Disk Usage : " << disk.used_disk_gb << " GB / " << disk.total_disk_gb << " GB (" << disk.disk_usage_percent << " %)" << std::endl;

        // Threshold Alert Checks
        if (cpu_perc > 90.0) {
            logger.warn("ResourceMonitor", "High CPU Utilization Detected: " + std::to_string(cpu_perc) + "%");
        }
        if (mem.ram_usage_percent > 85.0) {
            logger.warn("ResourceMonitor", "High RAM Usage Detected: " + std::to_string(mem.ram_usage_percent) + "%");
        }
    }

    // Display Sample Process List
    std::cout << "\n----------------------------------------------------------" << std::endl;
    std::cout << " Top Monitored Processes Sample:" << std::endl;
    std::cout << "----------------------------------------------------------" << std::endl;
    std::vector<ProcessDetails> processes = proc_mgr.get_running_processes_list();
    int count = 0;
    for (const auto& proc : processes) {
        std::cout << "  PID: " << std::setw(6) << proc.pid 
                  << " | State: " << proc.state 
                  << " | PPID: " << std::setw(6) << proc.ppid 
                  << " | Name: " << proc.name << std::endl;
        if (++count >= 5) break; // Display top 5 processes
    }

    logger.info("Main", "Stage 4 Prototype Sampling Completed Successfully.");
    std::cout << "\n[SentinelOS] Stage 4 Prototype Execution Finished." << std::endl;
    return 0;
}
