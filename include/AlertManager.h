#ifndef ALERT_MANAGER_H
#define ALERT_MANAGER_H

#include <string>
#include <vector>
#include "ProcessManager.h"
#include "ResourceMonitor.h"

/**
 * @class AlertManager
 * @brief Evaluates system resource thresholds and process state anomalies to trigger real-time alerts.
 */
class AlertManager {
private:
    double m_cpu_threshold_percent{90.0};
    double m_ram_threshold_percent{85.0};
    double m_disk_threshold_percent{90.0};

public:
    AlertManager(double cpu_thresh = 90.0, double ram_thresh = 85.0, double disk_thresh = 90.0);
    ~AlertManager() = default;

    // Evaluates CPU, RAM, and Disk metrics against configured threshold limits
    void check_resource_thresholds(double cpu_percent, double ram_percent, double disk_percent);

    // Inspects process list for Zombie ('Z') or Stopped ('T') states
    void check_process_anomalies(const std::vector<ProcessInfo>& process_list);
};

#endif // ALERT_MANAGER_H
