#include "App.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>

namespace Sentinel {

    SentinelApp::SentinelApp() : m_is_running(false) {}

    Status SentinelApp::initialize() {
        std::cout << "[SentinelApp] Initializing " << PLATFORM_NAME << " v" << VERSION << "..." << std::endl;
        
        m_resource_monitor = std::make_unique<ResourceMonitor>();
        std::cout << "[SentinelApp] ResourceMonitor initialized successfully." << std::endl;

        m_is_running = true;
        return Status::SUCCESS;
    }

    void SentinelApp::run() {
        if (!m_is_running || !m_resource_monitor) {
            std::cerr << "[SentinelApp] Error: Cannot run uninitialized application." << std::endl;
            return;
        }

        std::cout << "[SentinelApp] Running Milestone 2 Resource Monitoring Sampling..." << std::endl;
        std::cout << "----------------------------------------------------------" << std::endl;

        for (int sample = 1; sample <= 3; ++sample) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            double cpu = m_resource_monitor->calculate_cpu_usage();
            MemoryStats mem = m_resource_monitor->get_memory_stats();
            DiskStats disk = m_resource_monitor->get_disk_stats("/");

            std::cout << "[Sample #" << sample << "] "
                      << "CPU: " << std::fixed << std::setprecision(2) << cpu << "% | "
                      << "RAM: " << std::setprecision(1) << mem.used_ram_mb << " MB / " << mem.total_ram_mb << " MB (" << mem.ram_usage_percent << "%) | "
                      << "Disk: " << disk.used_disk_gb << " GB / " << disk.total_disk_gb << " GB (" << disk.disk_usage_percent << "%)"
                      << std::endl;
        }

        std::cout << "----------------------------------------------------------" << std::endl;
        std::cout << "[SentinelApp] Milestone 2 verification complete." << std::endl;
    }

    void SentinelApp::shutdown() {
        std::cout << "[SentinelApp] Shutting down application gracefully..." << std::endl;
        m_resource_monitor.reset();
        m_is_running = false;
    }

} // namespace Sentinel
