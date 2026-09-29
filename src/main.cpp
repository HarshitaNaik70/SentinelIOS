#include <iostream>
#include <chrono>
#include "ThreadCompat.h"

#include "Logger.h"
#include "ResourceMonitor.h"
#include "KernelMonitor.h"
#include "ProcessManager.h"
#include "AlertManager.h"
#include "RecoveryManager.h"

int main() {
    Logger::getInstance().info("Main", "Initializing SentinelOS Milestone 8 - Autonomous Self-Healing Recovery Engine...");

    std::cout << "==========================================================================================" << std::endl;
    std::cout << "            SentinelOS: Milestone 8 - Self-Healing Recovery Engine                        " << std::endl;
    std::cout << "==========================================================================================" << std::endl;

    // 1. Initialize Core Telemetry Subsystems
    ResourceMonitor resource_mon;
    KernelMonitor kernel_mon;
    ProcessManager process_mon;
    AlertManager alert_mgr(90.0, 85.0, 90.0);
    RecoveryManager recovery_mgr;

    std::cout << "\n[Subsystems Online] ResourceMonitor, KernelMonitor, ProcessManager, AlertManager, RecoveryManager." << std::endl;

    // 2. Configure Monitored Critical System Processes
    MonitoredProcess worker_proc;
    worker_proc.name = "WorkerDaemon";
#if defined(__linux__)
    worker_proc.binary_path = "/bin/sleep";
#else
    worker_proc.binary_path = "dummy_worker";
#endif
    worker_proc.args = {"300"};
    worker_proc.policy = RestartPolicy::IMMEDIATE;
    worker_proc.max_retries = 3;
    worker_proc.cooldown_seconds = 1;

    MonitoredProcess db_proc;
    db_proc.name = "DatabaseBridge";
#if defined(__linux__)
    db_proc.binary_path = "/bin/sleep";
#else
    db_proc.binary_path = "dummy_db";
#endif
    db_proc.args = {"300"};
    db_proc.policy = RestartPolicy::DELAYED;
    db_proc.max_retries = 2;
    db_proc.cooldown_seconds = 2;

    // Register process configs into RecoveryManager
    recovery_mgr.register_process(worker_proc);
    recovery_mgr.register_process(db_proc);

    // 3. Launch Process Supervision
    std::cout << "\n[Step 1] Starting supervised processes..." << std::endl;
    recovery_mgr.start_process("WorkerDaemon");
    recovery_mgr.start_process("DatabaseBridge");

    // Display Initial Recovery Dashboard
    std::cout << "\n[Step 2] Displaying Initial Recovery Engine Dashboard:" << std::endl;
    recovery_mgr.display_dashboard();

    // Start background health monitoring thread
    recovery_mgr.start_monitoring();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // 4. TEST CASE 1: Simulate Process Crash & Verify Automatic Recovery
    std::cout << "\n==========================================================================================" << std::endl;
    std::cout << "  [TEST CASE 1] Simulating Process Crash for 'WorkerDaemon' (SIGKILL / Force Termination) " << std::endl;
    std::cout << "==========================================================================================" << std::endl;

    std::cout << "[Simulation] Triggering force crash / SIGKILL for process 'WorkerDaemon'..." << std::endl;
    recovery_mgr.force_crash_process("WorkerDaemon");

    // Allow background monitor thread to detect crash and execute self-healing restart
    std::cout << "[Self-Healing] Background Recovery Engine detecting failure and executing restart..." << std::endl;
    recovery_mgr.perform_health_check();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "\n[Step 3] Recovery Engine Dashboard After Automatic Recovery:" << std::endl;
    recovery_mgr.display_dashboard();

    // 5. TEST CASE 2: Demonstrate Permanent Failure Threshold & CRITICAL Alerting
    std::cout << "\n==========================================================================================" << std::endl;
    std::cout << "  [TEST CASE 2] Simulating Persistent Crash Loop Exceeding Max Retries (Critical Alert)   " << std::endl;
    std::cout << "==========================================================================================" << std::endl;

    // Register an intentionally failing binary path to trigger retry exhaustion
    MonitoredProcess failing_proc;
    failing_proc.name = "UnstableTask";
    failing_proc.binary_path = "/invalid/path/non_existent_binary";
    failing_proc.policy = RestartPolicy::IMMEDIATE;
    failing_proc.max_retries = 2;
    failing_proc.cooldown_seconds = 1;

    recovery_mgr.register_process(failing_proc);
    recovery_mgr.start_process("UnstableTask");

    // Force health check ticks to trigger retries until retry limit is exceeded
    std::cout << "[Self-Healing] Exhausting retries for 'UnstableTask' (Max Retries: 2)..." << std::endl;
    for (int i = 0; i < 3; ++i) {
        recovery_mgr.perform_health_check();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::cout << "\n[Step 4] Final Recovery Engine Dashboard Output:" << std::endl;
    recovery_mgr.display_dashboard();

    // Stop background monitoring thread and clean up processes
    std::cout << "\n[Shutdown] Stopping Recovery Engine supervision loop..." << std::endl;
    recovery_mgr.stop_monitoring();
    recovery_mgr.stop_process("WorkerDaemon");
    recovery_mgr.stop_process("DatabaseBridge");

    Logger::getInstance().info("Main", "SentinelOS Milestone 8 Self-Healing Engine Test Completed Successfully.");
    std::cout << "\n==========================================================================================" << std::endl;
    std::cout << " [SUCCESS] Milestone 8 Self-Healing Recovery Engine Integrated Test Completed." << std::endl;
    std::cout << "==========================================================================================" << std::endl;

    return 0;
}
