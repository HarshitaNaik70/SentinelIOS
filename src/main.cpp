#include <iostream>
#include <chrono>
#include "ThreadCompat.h"
#include "ResourceMonitor.h"
#include "KernelMonitor.h"
#include "ProcessManager.h"
#include "Server.h"

int main() {
    std::cout << "==========================================================================================" << std::endl;
    std::cout << "           SentinelOS: Autonomous Health Monitoring & Telemetry Platform                  " << std::endl;
    std::cout << "==========================================================================================" << std::endl;

    // 1. Initialize Subsystems
    ResourceMonitor resource_mon;
    KernelMonitor kernel_mon;
    ProcessManager process_mon;

    std::cout << "\n[Subsystems Online] ResourceMonitor, KernelMonitor, ProcessManager initialized." << std::endl;

    // 2. Display System Summary
    kernel_mon.display_kernel_summary();

    // 3. Display Process Summary
    process_mon.display_process_summary();

    // 4. Start Multithreaded TCP Monitoring Server
    Server server(9090);
    if (!server.start()) {
        std::cerr << "[Main Error] Failed to start TCP Monitoring Server on port 9090." << std::endl;
        return 1;
    }

    std::cout << "\n[SentinelOS Active] Server listening on TCP port 9090." << std::endl;
    std::cout << "[Info] Press CTRL+C to terminate SentinelOS daemon.\n" << std::endl;

    // Keep server running in main loop
    while (server.is_running()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    server.stop();
    std::cout << "[SentinelOS Shutdown] Server daemon stopped cleanly." << std::endl;
    return 0;
}
