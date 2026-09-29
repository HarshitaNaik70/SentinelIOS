# SentinelOS: Autonomous Linux Health Monitoring, Process Management & Self-Healing Platform

**Academic Stage 4 Prototype Implementation Plan & Linux C++ Source Specification**

---

| Metadata | Details |
| :--- | :--- |
| **Project Name** | SentinelOS |
| **Full Title** | Autonomous Linux Health Monitoring, Process Management & Self-Healing Platform |
| **Author** | Harshita Naik (B.Tech Computer Science & Engineering, SOA University) |
| **Document Type** | Stage 4 Prototype Implementation Plan & Linux C++ Source Specification |
| **Target OS / Kernel** | Linux (Ubuntu / POSIX compliant) |
| **Implementation Language** | Modern C++ (C++17 / C++20) |
| **Document Version** | 4.0.0 (Stage 4 Submission) |
| **Status** | Approved Prototype Implementation Specification |

---

## Table of Contents

1. [Folder Structure](#1-folder-structure)
2. [C++ Class Structure](#2-c-class-structure)
3. [Header Files (Specification)](#3-header-files-specification)
4. [Source Files (Implementation)](#4-source-files-implementation)
5. [Step-by-Step Development Plan](#5-step-by-step-development-plan)
6. [Git Commit Plan](#6-git-commit-plan)
7. [Weekly Progress Log Format](#7-weekly-progress-log-format)
8. [Testing Strategy](#8-testing-strategy)
9. [Build Configuration (CMake & Makefile)](#9-build-configuration-cmake--makefile)

---

## 1. Folder Structure

The Stage 4 prototype establishes a clean, industry-standard C++ directory hierarchy separating header interfaces (`include/`), implementation sources (`src/`), build configuration (`CMakeLists.txt`, `Makefile`), and documentation (`docs/`).

```text
SentinelOS/
│
├── docs/                                    # Stage Documentation
│   ├── SentinelOS_Stage1_Documentation.md
│   ├── SentinelOS_Stage2_Documentation.md
│   ├── SentinelOS_Stage3_Design_Architecture.md
│   └── SentinelOS_Stage4_Prototype_Plan.md
│
├── include/                                  # C++ Header Interfaces
│   ├── ResourceMonitor.h                    # Module 1: Resource Monitoring
│   ├── ProcessManager.h                     # Module 2: Process Management
│   ├── KernelMonitor.h                      # Module 3: Kernel Information
│   └── AlertManager.h                       # Module 7: Logging & Alerts
│
├── src/                                      # C++ Source Implementations
│   ├── ResourceMonitor.cpp
│   ├── ProcessManager.cpp
│   ├── KernelMonitor.cpp
│   ├── AlertManager.cpp
│   └── main.cpp                             # Prototype Main Application Entry Point
│
├── CMakeLists.txt                            # CMake Build Configuration
├── Makefile                                  # GNU Makefile Automation
└── README.md                                 # Project Readme
```

---

## 2. C++ Class Structure

```text
                               +-------------------+
                               |   AlertManager    |
                               +-------------------+
                               | - log_file        |
                               | + log_info()      |
                               | + log_warn()      |
                               | + log_error()     |
                               +---------+---------+
                                         | (uses for logging)
        +--------------------------------+--------------------------------+
        |                                |                                |
        v                                v                                v
+-------------------+          +-------------------+          +-------------------+
|  ResourceMonitor  |          |  ProcessManager   |          |   KernelMonitor   |
+-------------------+          +-------------------+          +-------------------+
| - prev_idle_ticks |          | + get_processes() |          | + get_kernel_ver()|
| - prev_total_ticks|          | + get_proc_status()|         | + get_sys_info()  |
| + get_cpu_usage() |          | + kill_process()  |          | + get_uptime()    |
| + get_ram_usage() |          +-------------------+          +-------------------+
| + get_disk_usage()|
+-------------------+
```

---

## 3. Header Files (Specification)

### 3.1 `include/AlertManager.h`
Provides structured logging capabilities (Console and File).

```cpp
#ifndef ALERT_MANAGER_H
#define ALERT_MANAGER_H

#include <string>
#include <fstream>
#include <mutex>

enum class LogLevel {
    INFO,
    WARNING,
    CRITICAL
};

class AlertManager {
private:
    std::string m_log_file_path;
    std::ofstream m_log_file;
    std::mutex m_log_mutex;

    std::string level_to_string(LogLevel level);
    std::string get_current_timestamp();

public:
    explicit AlertManager(const std::string& log_file_path = "sentinel.log");
    ~AlertManager();

    void log(LogLevel level, const std::string& module, const std::string& message);
    void info(const std::string& module, const std::string& message);
    void warn(const std::string& module, const std::string& message);
    void error(const std::string& module, const std::string& message);
};

#endif // ALERT_MANAGER_H
```

### 3.2 `include/ResourceMonitor.h`
Monitors CPU utilization percentage, RAM metrics, and Disk storage usage.

```cpp
#ifndef RESOURCE_MONITOR_H
#define RESOURCE_MONITOR_H

#include <cstdint>
#include <string>

struct CpuTicks {
    uint64_t idle_ticks{0};
    uint64_t total_ticks{0};
};

struct MemoryInfo {
    double total_ram_mb{0.0};
    double free_ram_mb{0.0};
    double used_ram_mb{0.0};
    double ram_usage_percent{0.0};
};

struct DiskInfo {
    double total_disk_gb{0.0};
    double free_disk_gb{0.0};
    double used_disk_gb{0.0};
    double disk_usage_percent{0.0};
};

class ResourceMonitor {
private:
    CpuTicks m_prev_cpu_ticks;
    CpuTicks read_proc_stat_ticks();

public:
    ResourceMonitor();
    ~ResourceMonitor() = default;

    double get_cpu_usage_percent();
    MemoryInfo get_memory_info();
    DiskInfo get_disk_info(const std::string& mount_point = "/");
};

#endif // RESOURCE_MONITOR_H
```

### 3.3 `include/ProcessManager.h`
Scans running processes from `/proc`, retrieves status metrics, and sends signals.

```cpp
#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include <string>
#include <vector>

struct ProcessDetails {
    int pid{0};
    std::string name;
    char state{'R'};
    int ppid{0};
    uint64_t memory_kb{0};
};

class ProcessManager {
public:
    ProcessManager() = default;
    ~ProcessManager() = default;

    std::vector<int> get_all_pids();
    ProcessDetails get_process_details(int pid);
    std::vector<ProcessDetails> get_running_processes_list();
    bool send_signal_to_process(int pid, int signal_number);
};

#endif // PROCESS_MANAGER_H
```

### 3.4 `include/KernelMonitor.h`
Retrieves kernel release version, operating system release string, uptime, and load averages.

```cpp
#ifndef KERNEL_MONITOR_H
#define KERNEL_MONITOR_H

#include <string>

struct SystemKernelInfo {
    std::string kernel_version;
    std::string os_name;
    double uptime_seconds{0.0};
    double load_avg_1min{0.0};
    double load_avg_5min{0.0};
    double load_avg_15min{0.0};
    int total_processes{0};
};

class KernelMonitor {
public:
    KernelMonitor() = default;
    ~KernelMonitor() = default;

    SystemKernelInfo get_system_kernel_info();
    std::string get_kernel_version_string();
};

#endif // KERNEL_MONITOR_H
```

---

## 4. Source Files (Implementation)

### 4.1 `src/AlertManager.cpp`

```cpp
#include "AlertManager.h"
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

AlertManager::AlertManager(const std::string& log_file_path)
    : m_log_file_path(log_file_path) {
    m_log_file.open(m_log_file_path, std::ios::app);
    if (!m_log_file.is_open()) {
        std::cerr << "[AlertManager] Warning: Could not open log file " << m_log_file_path << std::endl;
    }
}

AlertManager::~AlertManager() {
    if (m_log_file.is_open()) {
        m_log_file.close();
    }
}

std::string AlertManager::level_to_string(LogLevel level) {
    switch (level) {
        case LogLevel::INFO:     return "INFO";
        case LogLevel::WARNING:  return "WARN";
        case LogLevel::CRITICAL: return "CRIT";
        default:                 return "INFO";
    }
}

std::string AlertManager::get_current_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_now{};
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&tm_now, &time_t_now);
#else
    localtime_r(&time_t_now, &tm_now);
#endif
    std::ostringstream ss;
    ss << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void AlertManager::log(LogLevel level, const std::string& module, const std::string& message) {
    std::lock_guard<std::mutex> lock(m_log_mutex);
    std::string timestamp = get_current_timestamp();
    std::string lvl_str = level_to_string(level);

    std::string log_line = "[" + timestamp + "] [" + lvl_str + "] [" + module + "] " + message;

    // Print to Console
    std::cout << log_line << std::endl;

    // Write to Log File
    if (m_log_file.is_open()) {
        m_log_file << log_line << std::endl;
    }
}

void AlertManager::info(const std::string& module, const std::string& message) {
    log(LogLevel::INFO, module, message);
}

void AlertManager::warn(const std::string& module, const std::string& message) {
    log(LogLevel::WARNING, module, message);
}

void AlertManager::error(const std::string& module, const std::string& message) {
    log(LogLevel::CRITICAL, module, message);
}
```

### 4.2 `src/ResourceMonitor.cpp`

```cpp
#include "ResourceMonitor.h"
#include <fstream>
#include <sstream>
#include <iostream>

#if defined(__linux__)
#include <sys/statvfs.h>
#endif

ResourceMonitor::ResourceMonitor() {
    m_prev_cpu_ticks = read_proc_stat_ticks();
}

CpuTicks ResourceMonitor::read_proc_stat_ticks() {
    CpuTicks ticks{0, 0};
    std::ifstream file("/proc/stat");
    if (!file.is_open()) {
        return ticks;
    }

    std::string line;
    if (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string cpu_label;
        uint64_t user, nice, system, idle, iowait, irq, softirq, steal;
        ss >> cpu_label >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

        ticks.idle_ticks = idle + iowait;
        ticks.total_ticks = user + nice + system + idle + iowait + irq + softirq + steal;
    }
    return ticks;
}

double ResourceMonitor::get_cpu_usage_percent() {
    CpuTicks curr_ticks = read_proc_stat_ticks();

    uint64_t idle_delta = curr_ticks.idle_ticks - m_prev_cpu_ticks.idle_ticks;
    uint64_t total_delta = curr_ticks.total_ticks - m_prev_cpu_ticks.total_ticks;

    m_prev_cpu_ticks = curr_ticks;

    if (total_delta == 0) {
        return 0.0;
    }

    double usage = (1.0 - (double)idle_delta / (double)total_delta) * 100.0;
    return (usage < 0.0) ? 0.0 : (usage > 100.0 ? 100.0 : usage);
}

MemoryInfo ResourceMonitor::get_memory_info() {
    MemoryInfo mem{0.0, 0.0, 0.0, 0.0};
    std::ifstream file("/proc/meminfo");
    if (!file.is_open()) {
        return mem;
    }

    std::string key;
    uint64_t value;
    std::string unit;

    uint64_t total_kb = 0;
    uint64_t available_kb = 0;

    while (file >> key >> value >> unit) {
        if (key == "MemTotal:") {
            total_kb = value;
        } else if (key == "MemAvailable:") {
            available_kb = value;
        }
    }

    if (total_kb > 0) {
        mem.total_ram_mb = (double)total_kb / 1024.0;
        mem.free_ram_mb = (double)available_kb / 1024.0;
        mem.used_ram_mb = mem.total_ram_mb - mem.free_ram_mb;
        mem.ram_usage_percent = (mem.used_ram_mb / mem.total_ram_mb) * 100.0;
    }

    return mem;
}

DiskInfo ResourceMonitor::get_disk_info(const std::string& mount_point) {
    DiskInfo disk{0.0, 0.0, 0.0, 0.0};
#if defined(__linux__)
    struct statvfs stat;
    if (statvfs(mount_point.c_str(), &stat) == 0) {
        uint64_t total_bytes = stat.f_blocks * stat.f_frsize;
        uint64_t free_bytes = stat.f_bavail * stat.f_frsize;
        uint64_t used_bytes = total_bytes - free_bytes;

        disk.total_disk_gb = (double)total_bytes / (1024.0 * 1024.0 * 1024.0);
        disk.free_disk_gb = (double)free_bytes / (1024.0 * 1024.0 * 1024.0);
        disk.used_disk_gb = (double)used_bytes / (1024.0 * 1024.0 * 1024.0);
        disk.disk_usage_percent = (disk.used_disk_gb / disk.total_disk_gb) * 100.0;
    }
#else
    // Fallback simulation for non-Linux host testing
    disk.total_disk_gb = 100.0;
    disk.used_disk_gb = 45.0;
    disk.free_disk_gb = 55.0;
    disk.disk_usage_percent = 45.0;
#endif
    return disk;
}
```

### 4.3 `src/ProcessManager.cpp`

```cpp
#include "ProcessManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

#if defined(__linux__)
#include <sys/types.h>
#include <signal.h>
#endif

namespace fs = std::filesystem;

std::vector<int> ProcessManager::get_all_pids() {
    std::vector<int> pids;
    if (!fs::exists("/proc")) {
        return pids;
    }

    for (const auto& entry : fs::directory_iterator("/proc")) {
        if (entry.is_directory()) {
            std::string filename = entry.path().filename().string();
            if (std::all_of(filename.begin(), filename.end(), ::isdigit)) {
                pids.push_back(std::stoi(filename));
            }
        }
    }
    return pids;
}

ProcessDetails ProcessManager::get_process_details(int pid) {
    ProcessDetails details;
    details.pid = pid;
    details.name = "Unknown";
    details.state = 'U';

    std::string stat_path = "/proc/" + std::to_string(pid) + "/stat";
    std::ifstream file(stat_path);
    if (!file.is_open()) {
        return details;
    }

    std::string line;
    if (std::getline(file, line)) {
        size_t open_paren = line.find('(');
        size_t close_paren = line.rfind(')');
        if (open_paren != std::string::npos && close_paren != std::string::npos) {
            details.name = line.substr(open_paren + 1, close_paren - open_paren - 1);
            
            std::string remaining = line.substr(close_paren + 2);
            std::istringstream ss(remaining);
            ss >> details.state >> details.ppid;
        }
    }
    return details;
}

std::vector<ProcessDetails> ProcessManager::get_running_processes_list() {
    std::vector<ProcessDetails> list;
    std::vector<int> pids = get_all_pids();
    for (int pid : pids) {
        ProcessDetails d = get_process_details(pid);
        if (d.pid > 0) {
            list.push_back(d);
        }
    }
    return list;
}

bool ProcessManager::send_signal_to_process(int pid, int signal_number) {
#if defined(__linux__)
    return (kill(pid, signal_number) == 0);
#else
    (void)pid;
    (void)signal_number;
    return true;
#endif
}
```

### 4.4 `src/KernelMonitor.cpp`

```cpp
#include "KernelMonitor.h"
#include <fstream>
#include <sstream>

#if defined(__linux__)
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#endif

SystemKernelInfo KernelMonitor::get_system_kernel_info() {
    SystemKernelInfo info;
    info.kernel_version = get_kernel_version_string();
    info.os_name = "Linux Generic";

#if defined(__linux__)
    struct sysinfo sys;
    if (sysinfo(&sys) == 0) {
        info.uptime_seconds = sys.uptime;
        info.load_avg_1min = (double)sys.loads[0] / 65536.0;
        info.load_avg_5min = (double)sys.loads[1] / 65536.0;
        info.load_avg_15min = (double)sys.loads[2] / 65536.0;
        info.total_processes = sys.procs;
    }
#else
    info.uptime_seconds = 3600.0;
    info.load_avg_1min = 0.25;
    info.load_avg_5min = 0.30;
    info.load_avg_15min = 0.20;
    info.total_processes = 120;
#endif
    return info;
}

std::string KernelMonitor::get_kernel_version_string() {
#if defined(__linux__)
    struct utsname buf;
    if (uname(&buf) == 0) {
        return std::string(buf.sysname) + " " + std::string(buf.release) + " (" + std::string(buf.machine) + ")";
    }
#endif
    return "Linux Kernel 6.x (x86_64)";
}
```

### 4.5 `src/main.cpp`

```cpp
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
```

---

## 5. Step-by-Step Development Plan

```text
Step 1: Setup Workspace & Directory Hierarchy
        Create `include/`, `src/`, `docs/`, `CMakeLists.txt`, `Makefile`.

Step 2: Implement Logging Subsystem (`AlertManager`)
        Build Console + File logging with timestamp generation.

Step 3: Implement Resource Monitoring (`ResourceMonitor`)
        Parse `/proc/stat` delta ticks for CPU and `/proc/meminfo` for RAM.

Step 4: Implement Process Monitoring (`ProcessManager`)
        Scan `/proc` directory, parse `/proc/[pid]/stat` for process metadata.

Step 5: Implement Kernel Information (`KernelMonitor`)
        Retrieve system uptime and load average via `sysinfo()` and `uname()`.

Step 6: Build Application Entry (`src/main.cpp`) & Verify Compilation
        Integrate modules, compile with `g++ -std=c++17`, execute test ticks.
```

---

## 6. Git Commit Plan

| Commit Order | Target Scope | Commit Message |
| :--- | :--- | :--- |
| **Commit 1** | Project Setup | `feat(setup): create Stage 4 folder structure, CMakeLists.txt and Makefile` |
| **Commit 2** | Logging Module | `feat(logger): implement AlertManager console and file logging` |
| **Commit 3** | Resource Module | `feat(resource): implement ResourceMonitor for CPU, RAM, and disk metrics` |
| **Commit 4** | Process Module | `feat(process): implement ProcessManager reading /proc pids and stat details` |
| **Commit 5** | Kernel Module | `feat(kernel): implement KernelMonitor using sysinfo and uname APIs` |
| **Commit 6** | Main Entry | `feat(main): integrate modules into Stage 4 prototype executable` |

---

## 7. Weekly Progress Log Format

```markdown
### SentinelOS - Weekly Progress Log

**Week Number**: Week 7 (Stage 4 Phase 1)  
**Developer**: Harshita Naik (B.Tech CSE, SOA University)  

#### Tasks Completed This Week:
- [x] Initialized project directory tree (`include/`, `src/`, `docs/`).
- [x] Created `AlertManager` logging interface with timestamp formatting.
- [x] Implemented `/proc/stat` CPU tick parser inside `ResourceMonitor`.

#### Blockers & Resolution:
- *Issue*: `statvfs()` requires POSIX headers (`<sys/statvfs.h>`).
- *Resolution*: Wrapped statvfs calls with `#if defined(__linux__)` platform guards.

#### Plan for Next Week:
- [ ] Complete `ProcessManager` `/proc` directory iteration.
- [ ] Implement `KernelMonitor` `sysinfo` data fetching.
- [ ] Verify prototype build with `make`.
```

---

## 8. Testing Strategy

| Test Case ID | Target Module | Test Description | Expected Result | Pass Criteria |
| :--- | :--- | :--- | :--- | :--- |
| **TC-RES-01** | `ResourceMonitor` | Verify CPU usage calculation between consecutive ticks. | Returns floating point percentage between 0.0% and 100.0%. | Value $\ge 0.0$ and $\le 100.0$. |
| **TC-RES-02** | `ResourceMonitor` | Verify RAM metrics extraction from `/proc/meminfo`. | Non-zero total RAM and available RAM values. | Total RAM > 0 MB. |
| **TC-PROC-01** | `ProcessManager` | Scan `/proc` directory for numerical PIDs. | Vector containing active PIDs (including PID 1 `systemd`/`init`). | PID 1 present in list. |
| **TC-KERN-01** | `KernelMonitor` | Fetch kernel release string via `uname()`. | Non-empty kernel release string (e.g. `Linux 6.x`). | String contains "Linux". |
| **TC-LOG-01** | `AlertManager` | Log INFO and WARN messages to file `sentinel.log`. | Messages written with timestamp to disk log file. | File exists & contains logs. |

---

## 9. Build Configuration (CMake & Makefile)

### 9.1 `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.20)
project(SentinelOS_Prototype VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

file(GLOB SOURCES "src/*.cpp")

add_executable(sentinel_prototype ${SOURCES})
```

### 9.2 `Makefile`

```makefile
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude
SRC = src/AlertManager.cpp src/ResourceMonitor.cpp src/ProcessManager.cpp src/KernelMonitor.cpp src/main.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = sentinel_prototype

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET) sentinel_stage4.log sentinel.log

.PHONY: all clean
```

---

## 10. Verification & Approval Sign-off

| Role | Name / Designation | Signature / Approval Status | Date |
| :--- | :--- | :--- | :--- |
| **Project Author** | **Harshita Naik** (B.Tech CSE, SOA University) | *Submitted for Review* | September 29, 2026 |
| **Faculty Supervisor** | Department of Computer Science & Engineering | *Pending Stage 4 Review* | Stage 4 Verification |
| **Project Reviewer** | Systems & Embedded Track Evaluator | *Pending Stage 4 Review* | Stage 4 Verification |
