# SentinelOS: Master System Integration & Demonstration Plan

**Platform Name:** SentinelOS (Autonomous Linux Health Monitoring, Process Management & Self-Healing Platform)  
**Author:** Harshita Naik (B.Tech CSE, SOA University)  
**Target OS:** Linux (Ubuntu 20.04 / 22.04 / 24.04 LTS)  
**Language & Standards:** Modern C++17, C11 (Kernel Driver), POSIX APIs, GNU Make, CMake  
**Release Version:** v1.0.0-final  

---

## 1. Final Project Folder Structure

```text
WiproProject/
├── CMakeLists.txt                      # Master CMake Build Configuration
├── Makefile                            # Master GNU Makefile
├── README.md                           # Comprehensive Technical Documentation
├── .gitignore                          # Git Artifact Ignore Rules
│
├── include/                            # Subsystem C++ Headers
│   ├── AlertManager.h                  # Threshold & Anomaly Evaluator
│   ├── App.h                           # System Bootstrapping Header
│   ├── Common.h                         # Global Constants & Status Enums
│   ├── KernelMonitor.h                 # Kernel & Hardware Information Collector
│   ├── Logger.h                        # Thread-Safe Singleton Logger
│   ├── ProcessManager.h                # /proc PID Scanner & Process Manager
│   ├── RecoveryManager.h               # Self-Healing Watchdog Engine
│   ├── ResourceMonitor.h               # Real-Time CPU, RAM & Disk Monitor
│   ├── Server.h                        # Multithreaded TCP Monitoring Server
│   └── ThreadCompat.h                  # Cross-Platform Threading Abstraction
│
├── src/                                # Core Engine Source Files
│   ├── AlertManager.cpp                # Resource/Process Anomaly Evaluator
│   ├── App.cpp                         # Application Bootstrapping Implementation
│   ├── KernelMonitor.cpp               # Linux System /proc & Uptime Telemetry
│   ├── Logger.cpp                       # Persistent Log File Writer (logs/sentinel.log)
│   ├── ProcessManager.cpp              # /proc Parser & POSIX Signal Manager
│   ├── RecoveryManager.cpp             # POSIX fork/exec Respawn & Policy Engine
│   ├── ResourceMonitor.cpp             # /proc/stat, /proc/meminfo & statvfs Parsers
│   ├── Server.cpp                      # TCP Socket Listener & Command Dispatcher
│   └── main.cpp                        # Main Server Daemon Entry Point
│
├── client/                             # Remote Telemetry Client Application
│   ├── Client.h                        # TCP Socket Client Header
│   ├── Client.cpp                      # Network Transmission Implementation
│   └── client_main.cpp                 # Interactive CLI Dashboard Client
│
├── driver/                             # Linux Kernel Subsystem
│   ├── Makefile                        # Linux Kbuild Kernel Module Makefile
│   ├── sentinel_driver.c               # Character Device Driver (/dev/sentinel)
│   ├── sentinel_ioctl.h                # IOCTL Command & Telemetry Struct Definitions
│   └── sentinel_test_app.c             # User-Space Driver Interoperability Test App
│
├── docs/                               # Complete Project Documentation Suite
│   ├── SentinelOS_Stage1_Documentation.md
│   ├── SentinelOS_Stage2_Documentation.md
│   ├── SentinelOS_Stage3_Design_Architecture.md
│   ├── SentinelOS_Stage4_Prototype_Plan.md
│   ├── SentinelOS_Stage5_Testing_Quality.md
│   ├── SentinelOS_Stage6_Final_Report.md
│   └── SentinelOS_Final_Integration_Plan.md
│
└── logs/                               # Runtime Output Directory
    └── sentinel.log                    # System Event Log File
```

---

## 2. End-to-End Build Process

### Prerequisites (Ubuntu Linux)
```bash
sudo apt update
sudo apt install -y build-essential gcc g++ cmake git linux-headers-$(uname -r)
```

### Target Compilation Commands

#### Option A: GNU Makefile Build
```bash
# 1. Build Server Daemon (sentinel_os) & CLI Client (sentinel_client)
make all

# 2. Build Linux Kernel Character Device Driver
cd driver
make

# 3. Build Driver User-Space Test Binary
gcc sentinel_test_app.c -o sentinel_test_app
cd ..
```

#### Option B: CMake Build
```bash
mkdir -p build && cd build
cmake ..
make -j$(nproc)
cd ..
```

---

## 3. Subsystem Integration Architecture

```mermaid
graph TD
    A[ResourceMonitor] -->|CPU, RAM, Disk Metrics| G[AlertManager]
    B[KernelMonitor] -->|Kernel Version, CPU Cores, Uptime| H[TCP Server :9090]
    C[ProcessManager] -->|PID Table, State 'Z', 'R', Memory| G
    C -->|POSX Signals / kill| F[RecoveryManager]
    D[Logger Singleton] <--|Thread-Safe Logging| A & B & C & F & G & H
    F -->|Self-Healing Respawn fork/exec| C
    F -->|WARNING / CRITICAL Alerts| G
    H -->|Telemetry Transmission| I[sentinel_client CLI]
    J[Kernel Driver /dev/sentinel] <-->|copy_to_user / copy_from_user / ioctl| K[sentinel_test_app]
```

---

## 4. System Verification & Test Cases

| Test ID | Subsystem | Test Objective | Inputs / Procedure | Expected Outcome |
| :--- | :--- | :--- | :--- | :--- |
| **TC-01** | ResourceMonitor | Evaluate CPU / RAM metrics | `/proc/stat` & `/proc/meminfo` read | CPU % and RAM MB correctly calculated |
| **TC-02** | ProcessManager | Scan processes & state | `scan_proc_pids()` across `/proc` | Active PIDs, names, states ('R','S','Z') listed |
| **TC-03** | AlertManager | Trigger High Usage Warning | CPU > 90% or RAM > 85% | Warning recorded in `logs/sentinel.log` |
| **TC-04** | TCP Server | Socket Command Processing | `GET_CPU`, `GET_MEMORY`, `GET_SYSTEM_STATUS` | Server returns formatted telemetry over port 9090 |
| **TC-05** | TCP Client | Remote Telemetry CLI | Launch `sentinel_client` | Interactive CLI fetches & displays live status |
| **TC-06** | RecoveryManager | Process Crash Self-Healing | Send `SIGKILL` to supervised PID | Engine detects crash, logs WARNING, respawns PID |
| **TC-07** | RecoveryManager | Max Retry Exhaustion | Attempt spawning invalid binary path | Retries exhaust (2/2), state set to `FAILED_PERMANENT`, CRITICAL alert triggered |
| **TC-08** | Character Driver | User-Kernel Interoperability | `open`, `write`, `read`, `ioctl` on `/dev/sentinel` | Data transferred safely via `copy_to_user`/`copy_from_user` |

---

## 5. Live Demonstration Scenarios

### Scenario 1: Autonomous Process Supervision & Crash Recovery
1. Launch `sentinel_os` server daemon.
2. Note initial PID of supervised target `WorkerDaemon` (e.g. PID 4001).
3. Trigger process crash using `force_crash_process("WorkerDaemon")` (or `kill -9 4001`).
4. Observe `RecoveryManager` detecting crash via non-blocking `waitpid()`.
5. Observe `AlertManager` WARNING log in console and `logs/sentinel.log`.
6. Verify automatic respawn with a new PID (e.g. PID 4003) and updated recovery dashboard timestamp.

### Scenario 2: Permanent Failure Escalation & CRITICAL Alerting
1. Register `UnstableTask` with an invalid binary path.
2. Start process supervision.
3. Observe initial failed launch attempts (Attempts 1/2 and 2/2).
4. Verify retry exhaustion, status transition to `FAILED_PERMANENT`, and generation of a `CRITICAL` alert requesting human operator intervention.

### Scenario 3: Remote Network Telemetry & TCP Client Monitoring
1. Run `./sentinel_os` in background terminal.
2. Run `./sentinel_client` in secondary terminal.
3. Select menu option `1` (`GET_CPU`), option `2` (`GET_MEMORY`), option `5` (`GET_PROCESSES`), and option `6` (`GET_SYSTEM_STATUS`).
4. Verify real-time network telemetry transmission over TCP port 9090.

### Scenario 4: Kernel Character Device Interoperability (`/dev/sentinel`)
1. Insert kernel module: `sudo insmod driver/sentinel_driver.ko`.
2. Set permissions: `sudo chmod 666 /dev/sentinel`.
3. Run user-space test app: `./driver/sentinel_test_app`.
4. Observe successful `open()`, telemetry `read()`, message payload `write()`, and IOCTL queries (`SENTINEL_IOCTL_GET_STATUS` and `SENTINEL_IOCTL_GET_KERN_MEM`).
5. Verify driver cleanup via `dmesg` logs.

---

## 6. Academic Viva Questions & Answers

### Q1: How does SentinelOS read CPU and Memory usage without external libraries?
**Answer:** SentinelOS directly parses the Linux `/proc` pseudo-filesystem. CPU usage is calculated by reading tick counters from `/proc/stat` across two sampling intervals ($\Delta \text{idle} / \Delta \text{total}$). Memory usage is calculated by parsing `MemTotal` and `MemAvailable` fields from `/proc/meminfo`.

### Q2: Explain the difference between `fork()` and `execvp()` in process supervision.
**Answer:** `fork()` creates a new child process by duplicating the parent's memory space and assigning a new PID. `execvp()` replaces the current process memory image with a new executable binary. `RecoveryManager` uses `fork()` + `execvp()` to respawn crashed background processes.

### Q3: Why is `WNOHANG` used with `waitpid()` in the recovery engine?
**Answer:** `WNOHANG` makes `waitpid()` non-blocking. If a child process has not exited, `waitpid()` returns `0` immediately instead of blocking the main thread, allowing the watchdog loop to continuously monitor all processes.

### Q4: Why can kernel code NOT directly access user-space pointer addresses?
**Answer:** User-space and kernel-space occupy separate virtual memory addresses protected by hardware privilege levels (Ring 3 vs Ring 0). Direct pointer dereferencing in kernel space can cause kernel panics, page faults, or security vulnerabilities (e.g. buffer overflows or privilege escalation). Kernel drivers must use `copy_to_user()` and `copy_from_user()` to validate memory access (`access_ok`) and safely transfer data across the memory barrier.

### Q5: What is the purpose of Major and Minor numbers in Linux character devices?
**Answer:** The **Major Number** identifies the specific kernel driver assigned to process file operations for that device. The **Minor Number** identifies the specific physical or virtual device instance managed by that driver. `alloc_chrdev_region()` dynamically allocates Major numbers to avoid conflicts with existing kernel modules.

---

## 7. Production Deployment Script (`sentinel_deploy.sh`)

```bash
#!/bin/bash
# SentinelOS Autonomous Platform Deployment Script
set -e

echo "[SentinelOS] Building SentinelOS Telemetry Engine & Driver..."
make all
cd driver && make && cd ..

echo "[SentinelOS] Loading Linux Kernel Subsystem..."
if lsmod | grep -q sentinel_driver; then
    sudo rmmod sentinel_driver
fi
sudo insmod driver/sentinel_driver.ko
sudo chmod 666 /dev/sentinel

echo "[SentinelOS] Starting SentinelOS Server Daemon..."
./sentinel_os &

echo "[SentinelOS] Deployment Completed Successfully."
```
