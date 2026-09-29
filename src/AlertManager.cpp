#include "AlertManager.h"
#include "Logger.h"
#include <iostream>

AlertManager::AlertManager(double cpu_thresh, double ram_thresh, double disk_thresh)
    : m_cpu_threshold_percent(cpu_thresh),
      m_ram_threshold_percent(ram_thresh),
      m_disk_threshold_percent(disk_thresh) {}

// Evaluates resource percentages against warning thresholds
void AlertManager::check_resource_thresholds(double cpu_percent, double ram_percent, double disk_percent) {
    if (cpu_percent > m_cpu_threshold_percent) {
        Logger::getInstance().warn("AlertManager", 
            "High CPU Utilization Warning: " + std::to_string(cpu_percent) + "% (Threshold: " + std::to_string(m_cpu_threshold_percent) + "%)");
    }

    if (ram_percent > m_ram_threshold_percent) {
        Logger::getInstance().warn("AlertManager", 
            "High RAM Usage Warning: " + std::to_string(ram_percent) + "% (Threshold: " + std::to_string(m_ram_threshold_percent) + "%)");
    }

    if (disk_percent > m_disk_threshold_percent) {
        Logger::getInstance().warn("AlertManager", 
            "High Disk Space Warning: " + std::to_string(disk_percent) + "% (Threshold: " + std::to_string(m_disk_threshold_percent) + "%)");
    }
}

// Scans process table for Zombie ('Z') or terminated/stopped processes
void AlertManager::check_process_anomalies(const std::vector<ProcessInfo>& process_list) {
    int zombie_count = 0;
    for (const auto& proc : process_list) {
        if (proc.state == 'Z') {
            zombie_count++;
            Logger::getInstance().error("AlertManager", 
                "Zombie Process Detected! PID: " + std::to_string(proc.pid) + " | Name: " + proc.name + " | PPID: " + std::to_string(proc.ppid));
        }
    }

    if (zombie_count > 0) {
        Logger::getInstance().critical("AlertManager", 
            "System Process Anomaly: Total " + std::to_string(zombie_count) + " Zombie Process(es) active!");
    }
}
