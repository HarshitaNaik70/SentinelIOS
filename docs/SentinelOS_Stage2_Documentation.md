# SentinelOS: Autonomous Linux Health Monitoring, Process Management & Self-Healing Platform

**Academic Stage 2 Project Documentation & Software Engineering Specification**

---

| Metadata | Details |
| :--- | :--- |
| **Project Name** | SentinelOS |
| **Full Title** | Autonomous Linux Health Monitoring, Process Management & Self-Healing Platform |
| **Author** | Harshita Naik (B.Tech Computer Science & Engineering, SOA University) |
| **Document Type** | Software Requirements Specification (SRS) & Product Requirements Document (PRD) |
| **Target OS / Kernel** | Linux Kernel 5.x / 6.x (x86_64 / ARM64) |
| **Implementation Languages** | C++20 (User-Space Daemon & Client), C11 (Kernel Module & POSIX Syscalls) |
| **Document Version** | 2.0.0 (Stage 2 Submission) |
| **Status** | Approved Requirements & Engineering Plan |

---

## Table of Contents

1. [Functional Requirements](#1-functional-requirements)
2. [Non-Functional Requirements](#2-non-functional-requirements)
3. [Product Requirements Document (PRD)](#3-product-requirements-document-prd)
4. [Module Breakdown](#4-module-breakdown)
5. [Features List & Matrix](#5-features-list--matrix)
6. [Project Deliverables](#6-project-deliverables)
7. [Development Timeline](#7-development-timeline)
8. [Milestone Planning](#8-milestone-planning)
9. [Risk Analysis & Mitigation](#9-risk-analysis--mitigation)
10. [Resource Requirements](#10-resource-requirements)
11. [Software and Hardware Requirements](#11-software-and-hardware-requirements)
12. [Git Branching & Release Strategy](#12-git-branching--release-strategy)

---

## 1. Functional Requirements

Functional requirements define the core operational behaviors, input handling, state transitions, and expected outputs of the SentinelOS platform across its seven primary sub-systems.

### 1.1 Module 1: Linux System Resource Monitoring

| Requirement ID | Requirement Name | Description | Inputs | Outputs / Behavior | Priority |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **FR-RES-01** | CPU Usage Tracking | The system shall compute per-core and total system CPU utilization percentages by reading `/proc/stat`. | Delta ticks (`user`, `system`, `nice`, `idle`, `iowait`, `irq`) | Floating-point percentage values per core; updated on tick. | **P1 (Must)** |
| **FR-RES-02** | RAM & Swap Tracking | The system shall parse `/proc/meminfo` to calculate total, used, available physical memory, and swap space. | `/proc/meminfo` key-value buffer | Struct containing exact memory metrics in MB and utilization %. | **P1 (Must)** |
| **FR-RES-03** | Disk Storage & I/O | The system shall inspect storage filesystem utilization via `statvfs()` and calculate read/write data transfer rates. | Mount point path (`/`), `/proc/diskstats` | Bytes read/written per sec; total/free storage in GB. | **P1 (Must)** |
| **FR-RES-04** | Network Interface Stats | The system shall monitor network interfaces via `/proc/net/dev` to track received/transmitted bandwidth, packets, and errors. | Network device name (e.g., `eth0`, `wlan0`) | Rx/Tx throughput (KB/s), packet drop counters. | **P2 (High)** |

### 1.2 Module 2: Process Monitoring and Management

| Requirement ID | Requirement Name | Description | Inputs | Outputs / Behavior | Priority |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **FR-PROC-01** | Process Tree Construction | The system shall scan `/proc/[pid]/stat` to construct dynamic Parent-Child process graph topologies (`PPID` $\rightarrow$ `PID`). | List of numerical subdirectories in `/proc` | Hierarchical Process Tree data structure. | **P1 (Must)** |
| **FR-PROC-02** | Process State Tracking | The system shall identify the execution state of monitored processes (`R`, `S`, `D`, `Z`, `T`). | Process status character from `/proc/[pid]/stat` | Enumerated state type; alerts on zombie/uninterruptible sleep states. | **P1 (Must)** |
| **FR-PROC-03** | POSIX Signal Delivery | The system shall allow authorized operators to send signals (`SIGTERM`, `SIGKILL`, `SIGSTOP`, `SIGCONT`) to target PIDs. | Target PID, signal integer code | Syscall `kill(pid, sig)` status code return. | **P1 (Must)** |
| **FR-PROC-04** | Resource Limit Enforcement | The system shall query and set process resource bounds (`RLIMIT_NOFILE`, `RLIMIT_AS`, `RLIMIT_CPU`) dynamically. | Target PID, `rlimit` struct bounds | Syscall `setrlimit()` execution feedback. | **P2 (High)** |

### 1.3 Module 3: Linux Kernel Information Monitoring

| Requirement ID | Requirement Name | Description | Inputs | Outputs / Behavior | Priority |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **FR-KERN-01** | Kernel Sysinfo & Load | The system shall fetch system uptime, machine architecture, and 1/5/15-minute load averages via `sysinfo()`. | Kernel syscall `sysinfo()` | Struct with uptime seconds, load averages, process counts. | **P1 (Must)** |
| **FR-KERN-02** | LKM Status Tracking | The system shall inspect loaded kernel modules by reading `/proc/modules`. | `/proc/modules` stream | List of dynamic LKMs, memory size, dependency usage counts. | **P2 (High)** |
| **FR-KERN-03** | Sysctl Inspection | The system shall query critical sysctl parameters (`kernel.ostype`, `kernel.hostname`, `kernel.threads-max`). | Filesystem paths in `/proc/sys/kernel/` | Key-value mapping of current kernel configuration states. | **P3 (Medium)** |

### 1.4 Module 4: Virtual Character Device Driver (`/dev/sentinel`)

| Requirement ID | Requirement Name | Description | Inputs | Outputs / Behavior | Priority |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **FR-DRV-01** | Dynamic Device Registration | The LKM shall allocate dynamic major/minor numbers and register character device `/dev/sentinel`. | Module `init()` invocation | Device node creation in `/dev` with file permissions `0660`. | **P1 (Must)** |
| **FR-DRV-02** | In-Kernel Ring Buffer | The driver shall maintain an atomic circular ring buffer in kernel RAM to log low-level driver events. | Event payload write calls from kernel space | Zero-copy / minimal-copy buffer stream accessible via `read()`. | **P1 (Must)** |
| **FR-DRV-03** | Custom IOCTL Channel | The driver shall implement `unlocked_ioctl` handling for kernel memory retrieval, status query, and ring reset. | `ioctl(fd, cmd, arg)` from user-space | Copy data safely between user and kernel space via `copy_to_user()`. | **P1 (Must)** |
| **FR-DRV-04** | Concurrency Locking | The driver shall synchronize file operations and ring buffer updates using kernel mutexes/spinlocks. | Concurrent user-space system calls | Data race prevention; thread-safe execution in multi-core environment. | **P1 (Must)** |

### 1.5 Module 5: Client-Server Monitoring Architecture

| Requirement ID | Requirement Name | Description | Inputs | Outputs / Behavior | Priority |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **FR-NET-01** | Multithreaded TCP Server | The daemon shall operate a TCP socket server on port `9090` accepting incoming client connections. | Socket bind/listen requests on IPv4 `0.0.0.0:9090` | Non-blocking client sockets managed via thread pool or `epoll`. | **P1 (Must)** |
| **FR-NET-02** | Wire Telemetry Protocol | The server shall stream structured JSON/Binary payload packets containing system stats to connected clients. | Periodic timer triggers (e.g., 1 sec interval) | Serialized metric payloads sent over active socket file descriptors. | **P1 (Must)** |
| **FR-NET-03** | Interactive CLI Dashboard | The client component (`sentinel-cli`) shall connect to the server and render real-time health dashboards in terminal. | TCP stream input from server | ANSI terminal formatted display showing CPU bars, RAM usage, process list. | **P1 (Must)** |

### 1.6 Module 6: Self-Healing Process Recovery

| Requirement ID | Requirement Name | Description | Inputs | Outputs / Behavior | Priority |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **FR-HEAL-01** | Process Supervision Registration | The daemon shall allow critical application binaries to be registered for automatic crash supervision. | Binary path, command-line flags, max restart count | Internal watchdog tracking entry. | **P1 (Must)** |
| **FR-HEAL-02** | Failure & Crash Detection | The watchdog shall trap child process termination signals (`SIGCHLD`) and monitor exit statuses via `waitpid()`. | Signal interrupts (`SIGSEGV`, `SIGABRT`, `SIGBUS`, non-zero exit) | Immediate failure event trigger; logging of failure signal code. | **P1 (Must)** |
| **FR-HEAL-03** | Automated Process Respawning | Upon detecting an unhandled process termination, the engine shall re-launch the process via `fork()` and `execvp()`. | Registered binary path & argv array | Re-creation of process with fresh PID within < 100ms. | **P1 (Must)** |
| **FR-HEAL-04** | Crash Loop Protection | The system shall track restart frequency and execute exponential backoff if a process continuously crashes. | Restart attempt timestamp history | Temporary suspension of restarts if threshold (e.g., 5 crashes in 30s) is exceeded. | **P2 (High)** |

### 1.7 Module 7: Logging and Alert System

| Requirement ID | Requirement Name | Description | Inputs | Outputs / Behavior | Priority |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **FR-LOG-01** | Severity Level Classification | The system shall categorize all log events into three distinct levels: `INFO`, `WARNING`, and `CRITICAL`. | Log event triggers from resource/process/driver modules | Formatted log record containing timestamp, level, module, and message. | **P1 (Must)** |
| **FR-LOG-02** | Multi-Sink Distribution | The logging subsystem shall output log records simultaneously to Console, File (`/var/log/sentinel.log`), and `syslog`. | Formatted log message string | Synchronous/Asynchronous write to file descriptor, syslog socket, and stdout. | **P1 (Must)** |
| **FR-LOG-03** | Alert Threshold Triggers | The system shall automatically raise `CRITICAL` alerts when CPU exceeds 90%, RAM exceeds 85%, or a process crashes. | Evaluated system resource values | Immediate alert emission to log file and client socket stream. | **P1 (Must)** |

---

## 2. Non-Functional Requirements

Non-Functional Requirements (NFRs) specify performance constraints, resource efficiency targets, security protocols, system reliability standards, and architectural quality attributes.

### 2.1 Performance & Timing Requirements

| NFR ID | Attribute | Specification Metric | Verification Method |
| :--- | :--- | :--- | :--- |
| **NFR-PERF-01** | Fault Detection Latency | The self-healing engine shall detect a process crash (`SIGSEGV`, `SIGABRT`) in **< 100 milliseconds**. | Automated signal injection benchmark test. |
| **NFR-PERF-02** | Recovery Latency | Automated process re-spawning (`fork` + `execvp`) shall complete within **< 200 milliseconds** of detection. | Timestamped signal log delta comparison. |
| **NFR-PERF-03** | Driver IOCTL Latency | Executing a kernel telemetry `ioctl` query shall consume **< 5 microseconds** per invocation. | High-resolution timer (`std::chrono` / `ktime_get`). |
| **NFR-PERF-04** | Socket Sampling Interval | Telemetry push update interval to connected CLI clients shall be configurable between **100ms and 5000ms**. | Client packet timestamp delta checking. |

### 2.2 Resource Efficiency & Footprint

| NFR ID | Attribute | Specification Metric | Verification Method |
| :--- | :--- | :--- | :--- |
| **NFR-RES-01** | Daemon Memory Footprint | The user-space daemon (`sentineld`) shall consume **< 15 MB RAM** (Resident Set Size). | `/proc/[pid]/statm` and `valgrind --tool=massif`. |
| **NFR-RES-02** | Daemon CPU Overhead | Daemon CPU utilization shall remain **< 1.5%** of a single CPU core under normal sampling workload. | `top` / `/proc/stat` monitoring during execution. |
| **NFR-RES-03** | LKM Ring Buffer Size | In-kernel circular ring buffer memory allocation shall be statically bounded to **< 64 KB**. | Kernel memory allocation audit (`kmalloc` size). |

### 2.3 Reliability, Availability & Security

| NFR ID | Attribute | Specification Metric | Verification Method |
| :--- | :--- | :--- | :--- |
| **NFR-REL-01** | Continuous Availability | The system daemon shall maintain **99.99% operational uptime** without internal memory leaks or deadlocks. | 72-hour stress testing with `valgrind --leak-check=full`. |
| **NFR-SEC-01** | Device File Access Control | The `/dev/sentinel` character device node shall enforce strict file mode permissions (`0660`, owner `root`). | POSIX permission check (`ls -l /dev/sentinel`). |
| **NFR-SEC-02** | Input Sanitization | All socket protocol requests and IPC commands shall undergo byte-level bounds and validation checks. | Fuzz testing socket commands with invalid payloads. |
| **NFR-PORT-01** | OS & Architecture Target | The platform shall compile cleanly on GCC 11+ for **x86_64** and **ARM64** Linux Kernels 5.x and 6.x. | Multi-architecture GCC compilation check. |

---

## 3. Product Requirements Document (PRD)

### 3.1 Target User Personas

1. **System Administrator / SRE (Site Reliability Engineer)**:
   * *Needs*: Centralized, real-time observability of server CPU, RAM, and process states with automated alerting.
   * *Pain Points*: High overhead of enterprise monitoring agents; delayed manual service restarts during off-hours.

2. **Embedded Systems Developer**:
   * *Needs*: Ultra-lightweight resource daemon that operates on resource-constrained ARM boards (e.g., Raspberry Pi, Jetson Nano).
   * *Pain Points*: Heavyweight Python/Go monitoring agents consuming excessive RAM/CPU; lack of direct kernel ring-buffer event streaming.

3. **Academic / Systems Researcher**:
   * *Needs*: Clean, modular codebase showcasing hybrid Linux kernel device driver and user-space C++ system programming techniques.
   * *Pain Points*: Lack of open-source reference implementations combining character drivers, POSIX signals, and self-healing watchdogs.

### 3.2 User Stories & Acceptance Criteria

```
+-------------------------------------------------------------------------------------------------------+
| User Story ID: US-01 - Real-Time Resource Observability                                               |
+-------------------------------------------------------------------------------------------------------+
| AS A System Administrator,                                                                            |
| I WANT TO view real-time system resource metrics (CPU, RAM, Disk, Network) on a terminal CLI,          |
| SO THAT I can monitor system health without installing heavy GUI applications.                       |
|                                                                                                       |
| Acceptance Criteria:                                                                                  |
| 1. CLI dashboard connects to `sentineld` daemon over TCP socket within 1 second.                     |
| 2. CPU usage is displayed with visual progress bars updated every 1000ms.                             |
| 3. Memory metrics show exact Total, Free, and Available values in Megabytes.                          |
+-------------------------------------------------------------------------------------------------------+
```

```
+-------------------------------------------------------------------------------------------------------+
| User Story ID: US-02 - Autonomous Process Self-Healing                                                |
+-------------------------------------------------------------------------------------------------------+
| AS AN Embedded Software Developer,                                                                    |
| I WANT TO register critical daemon binaries with the SentinelOS supervisor watchdog,                 |
| SO THAT if a daemon encounters a crash (e.g., SIGSEGV), it is automatically restarted instantly.     |
|                                                                                                       |
| Acceptance Criteria:                                                                                  |
| 1. Process failure is trapped via `SIGCHLD` signal within 100ms of termination.                       |
| 2. Failed process is re-spawned with identical command-line arguments via `fork()` and `execvp()`.    |
| 3. Recovery event is logged to `/var/log/sentinel.log` with timestamp and old/new PIDs.              |
| 4. Crash loop protection triggers backoff if process crashes > 5 times within 30 seconds.            |
+-------------------------------------------------------------------------------------------------------+
```

```
+-------------------------------------------------------------------------------------------------------+
| User Story ID: US-03 - Kernel Telemetry Stream via Character Device                                  |
+-------------------------------------------------------------------------------------------------------+
| AS A Systems Engineering Student,                                                                     |
| I WANT TO query kernel memory statistics directly from `/dev/sentinel` using custom IOCTL commands,  |
| SO THAT I can bypass slow text file parsing in `/proc` for critical low-latency telemetry.            |
|                                                                                                       |
| Acceptance Criteria:                                                                                  |
| 1. Kernel driver `sentinel_driver.ko` loads without kernel panic or warnings (`dmesg` clean).         |
| 2. Executing `SENTINEL_IOCTL_GET_KERN_MEM` returns valid kernel struct data in < 5 microseconds.      |
| 3. Unprivileged user access to `/dev/sentinel` is blocked with `EACCES` permission denied.             |
+-------------------------------------------------------------------------------------------------------+
```

### 3.3 Product Assumptions & Dependencies

* **Linux Kernel Environment**: Target system must run a Linux Kernel version $\ge$ 5.4 with root privileges required to load the kernel module (`insmod`).
* **Toolchain Availability**: System must have `gcc` (v11+), `g++` (supporting `-std=c++20`), `make`, and kernel development headers (`linux-headers-$(uname -r)`).
* **POSIX Compliance**: Operating environment strictly conforms to standard IEEE Std 1003.1 (POSIX) system call interfaces.

---

## 4. Module Breakdown

SentinelOS is partitioned into seven distinct, decoupled functional modules.

```
+-----------------------------------------------------------------------------------+
|                              SentinelOS System Architecture                       |
+-----------------------------------------------------------------------------------+
|  [ Module 1: Resource Monitor ] <--------+                                        |
|  [ Module 2: Process Manager  ] <----+   |                                        |
|  [ Module 3: Kernel Monitor   ] <--+ |   | (Internal C++ API Data Pipeline)       |
|                                    | |   |                                        |
|  +---------------------------------+-+---+-------------------------------------+  |
|  | Module 5: Client-Server Socket IPC Engine (TCP Server 9090 / CLI Dashboard)  |  |
|  +---------------------------------+-+---+-------------------------------------+  |
|                                    | |   |                                        |
|  [ Module 6: Self-Healing Watchdog] <----+                                        |
|  [ Module 7: Multi-Sink Logger    ] <------------------------------------------+  |
|                                                                                   |
|  ==============================================================================   |
|  [ Module 4: Virtual Character Driver /dev/sentinel ] (Linux Kernel Space)        |
+-----------------------------------------------------------------------------------+
```

### 4.1 Detailed Module Descriptions

1. **Module 1: Resource Monitoring Engine (`ResourceMonitor`)**:
   * Responsible for periodic sampling of hardware resource states.
   * Reads `/proc/stat` for CPU ticks and calculates per-core delta percentages.
   * Parses `/proc/meminfo` for physical and swap memory allocations.
   * Executes `statvfs()` syscall for disk filesystem utilization and monitors `/proc/net/dev` for network bandwidth.

2. **Module 2: Process Management Engine (`ProcessManager`)**:
   * Scans `/proc` filesystem to construct dynamic process trees mapping `PPID` to `PID`.
   * Tracks process execution states (`Running`, `Sleeping`, `Disk Sleep`, `Zombie`, `Stopped`).
   * Provides interface for sending POSIX signals (`SIGTERM`, `SIGKILL`, `SIGSTOP`, `SIGCONT`) and adjusting priority via `nice()`.
   * Enforces resource limits via `setrlimit()` (`RLIMIT_NOFILE`, `RLIMIT_AS`, `RLIMIT_CPU`).

3. **Module 3: Kernel Information Monitoring Engine (`KernelMonitor`)**:
   * Fetches kernel version string, machine architecture, and boot timestamps via `sysinfo()`.
   * Monitors dynamic list of loaded kernel modules in `/proc/modules`.
   * Inspects system sysctl configuration variables in `/proc/sys/kernel/`.

4. **Module 4: Virtual Character Device Driver (`sentinel_driver`)**:
   * C-based Loadable Kernel Module (LKM) providing `/dev/sentinel` character device interface.
   * Implements custom `file_operations`: `open`, `read`, `write`, `unlocked_ioctl`, `release`.
   * Manages a thread-safe circular ring buffer in kernel memory for direct event streaming.
   * Provides `ioctl` handling for kernel memory metrics and driver reset primitives.

5. **Module 5: Client-Server Telemetry Infrastructure (`Server` & `Client`)**:
   * Multithreaded C++ socket server listening on TCP port `9090`.
   * Uses non-blocking socket I/O multiplexing to serve real-time telemetry streams.
   * Features an interactive C++ CLI client dashboard (`sentinel-cli`) rendering terminal UI bars and status tables.

6. **Module 6: Autonomous Self-Healing Subsystem (`RecoveryManager`)**:
   * Operates an active supervisor watchdog engine over registered application binaries.
   * Traps `SIGCHLD` signals asynchronously and inspects process exit statuses via `waitpid()`.
   * Executes zero-downtime process re-spawning via `fork()` and `execvp()`.
   * Implements exponential backoff to guard against rapid crash loops.

7. **Module 7: Logging and Alert Subsystem (`AlertManager`)**:
   * Multi-sink logging pipeline supporting `INFO`, `WARN`, and `CRITICAL` alert levels.
   * Streams log records concurrently to standard output, rotating disk log files (`/var/log/sentinel.log`), and `syslog`.
   * Evaluates resource thresholds to trigger instant warning alerts when CPU or RAM limits are breached.

---

## 5. Features List & Matrix

The feature matrix categorizes all capabilities, priority levels, implementation complexity, and target milestone stages.

| Feature ID | Feature Name | Description | Priority | Complexity | Target Stage |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **FEAT-01** | CPU Utilization Parser | Per-core and total CPU percentage calculator using `/proc/stat`. | **P1** | Low | Stage 4 |
| **FEAT-02** | Memory & Swap Tracker | RAM/Swap metrics extractor from `/proc/meminfo`. | **P1** | Low | Stage 4 |
| **FEAT-03** | Disk & Network Monitor | Disk capacity via `statvfs()` and network I/O from `/proc/net/dev`. | **P2** | Medium | Stage 4 |
| **FEAT-04** | Process Tree Visualizer | Dynamic PPID-PID process tree hierarchy generator. | **P1** | Medium | Stage 4 |
| **FEAT-05** | POSIX Signal Controller | Signal delivery dispatch (`kill`) and priority modulation (`nice`). | **P1** | Low | Stage 4 |
| **FEAT-06** | Resource Limit Manager | Dynamic process limit setter using `setrlimit()`. | **P2** | Medium | Stage 4 |
| **FEAT-07** | Kernel Sysinfo Inspector| System uptime, load average, and sysctl parameter reader. | **P1** | Low | Stage 4 |
| **FEAT-08** | Dynamic LKM Tracker | Loaded module parser reading `/proc/modules`. | **P2** | Low | Stage 4 |
| **FEAT-09** | Character Driver Module | LKM character device registration `/dev/sentinel`. | **P1** | High | Stage 6 |
| **FEAT-10** | Kernel Ring Buffer | In-kernel circular ring buffer for driver event logging. | **P1** | High | Stage 6 |
| **FEAT-11** | Driver IOCTL Interface | Custom `ioctl` commands for low-latency kernel metric extraction. | **P1** | High | Stage 6 |
| **FEAT-12** | TCP Socket Daemon | Multithreaded TCP server on port 9090 streaming JSON metrics. | **P1** | Medium | Stage 6 |
| **FEAT-13** | Terminal CLI Dashboard | Interactive terminal client rendering resource bars and alerts. | **P1** | Medium | Stage 6 |
| **FEAT-14** | Autonomous Watchdog | `SIGCHLD` signal handler & failure state trap engine. | **P1** | High | Stage 5 |
| **FEAT-15** | Auto Process Respawn | Automated binary re-spawning via `fork()` + `execvp()`. | **P1** | Medium | Stage 5 |
| **FEAT-16** | Crash Loop Protection | Exponential backoff algorithm to prevent infinite restart loops. | **P2** | Medium | Stage 5 |
| **FEAT-17** | Multi-Sink Logger | Logger outputting to Console, File (`var/log/sentinel.log`), `syslog`. | **P1** | Low | Stage 4 |
| **FEAT-18** | Threshold Alert Engine | Automatic alert trigger on CPU > 90% or RAM > 85%. | **P1** | Low | Stage 4 |

---

## 6. Project Deliverables

Upon completion of all development stages, SentinelOS will yield the following hardware, software, and documentation deliverables:

```
c:\Users\harsh\OneDrive\Desktop\WiproProject\
├── docs/
│   ├── SentinelOS_Stage1_Documentation.md      # Stage 1 Architecture & Specs
│   ├── SentinelOS_Stage2_Documentation.md      # Stage 2 PRD & Software Requirements
│   ├── SentinelOS_Stage3_Design_Architecture.md # Stage 3 Architecture & UML Diagrams
│   └── SentinelOS_Final_Project_Report.pdf     # Stage 6 Final Presentation & Submission
│
├── src/                                        # Core User-Space Daemon Source Code
│   ├── ResourceMonitor.cpp                     # Module 1 Implementation
│   ├── ProcessManager.cpp                      # Module 2 Implementation
│   ├── KernelMonitor.cpp                       # Module 3 Implementation
│   ├── AlertManager.cpp                        # Module 7 Implementation
│   ├── RecoveryManager.cpp                     # Module 6 Implementation
│   ├── Server.cpp                              # Module 5 Server Implementation
│   └── main.cpp                                # Daemon Main Entry Point
│
├── include/                                    # C++ Header Interfaces
│   ├── ResourceMonitor.h
│   ├── ProcessManager.h
│   ├── KernelMonitor.h
│   ├── AlertManager.h
│   ├── RecoveryManager.h
│   └── Server.h
│
├── client/                                     # Remote Monitoring Client
│   └── Client.cpp                              # Interactive Terminal CLI Dashboard
│
├── driver/                                     # Linux Kernel Character Driver
│   ├── sentinel_driver.c                       # Module 4 LKM Source Code
│   ├── sentinel_ioctl.h                        # Shared IOCTL Header Interface
│   └── Makefile                                # Kbuild Out-of-Tree Driver Build
│
├── tests/                                      # Automated Test Suite
│   ├── test_resource_monitor.cpp
│   ├── test_self_healing.cpp
│   └── test_driver_ioctl.cpp
│
├── logs/                                       # Execution Log Directory
│   └── sentinel.log
│
├── CMakeLists.txt                              # CMake Build Configuration for C++
├── Makefile                                    # Top-Level Master Build Script
├── README.md                                   # Project Overview & Setup Guide
└── .gitignore                                  # Git Artifact Exclusions
```

---

## 7. Development Timeline

The SentinelOS platform development plan spans six structured project stages across a 12-week timeline.

```text
+-----------------------------------------------------------------------------------------------+
| Timeline | Stage Focus                          | Core Deliverables                           |
+-----------------------------------------------------------------------------------------------+
| Weeks 1-2| Stage 1: Introduction & Specification| High-level Architecture, Scope & Specs      |
| Weeks 3-4| Stage 2: Requirements & PRD Creation  | SRS, PRD, Risk Analysis, Timeline Plan      |
| Weeks 5-6| Stage 3: System Architecture & UML   | Detailed Class, Sequence & Activity Diagrams|
| Weeks 7-8| Stage 4: Core Prototype Development  | Resource Monitor, Process Manager, Logger   |
| Weeks 9-10| Stage 5: Integration & Self-Healing | Recovery Manager, Signal Traps, Backoff     |
| Weeks 11-12| Stage 6: Driver & Socket Completion| /dev/sentinel LKM, TCP CLI, Final Report    |
+-----------------------------------------------------------------------------------------------+
```

---

## 8. Milestone Planning

Milestones define critical project progress verification gates. Each milestone requires passing specific acceptance checks before transitioning to subsequent stages.

| Milestone ID | Target Week | Milestone Name | Key Deliverable / Gate | Verification Criteria |
| :--- | :--- | :--- | :--- | :--- |
| **MS-1** | Week 2 | Stage 1 Approval | Project Specification & High-Level Scope Document | Document sign-off; GitHub repository setup. |
| **MS-2** | Week 4 | Stage 2 Requirements Gate | SRS, PRD, Module Breakdown, Feature Matrix | Requirements completeness review; PRD approval. |
| **MS-3** | Week 6 | Stage 3 Design Blueprint | System Architecture & UML Diagram Suite | Class/Sequence diagram review & module interface check. |
| **MS-4** | Week 8 | Stage 4 Prototype Gate | Working User-Space Resource & Process Monitor | Successful parsing of `/proc` stats; zero memory leaks. |
| **MS-5** | Week 10 | Stage 5 Self-Healing Engine | Autonomous Recovery Watchdog & Signal Handler | Verified process auto-restart latency < 100ms. |
| **MS-6** | Week 12 | Stage 6 Driver & Final Release| Kernel Character Driver, TCP CLI, Final PDF Report | Successful `insmod`, `ioctl` execution, and TCP client UI. |

---

## 9. Risk Analysis & Mitigation

Risk management identifies potential technical hurdles in kernel driver development, concurrency handling, and self-healing signal management, along with proactive mitigation strategies.

| Risk ID | Risk Category | Risk Description | Probability | Impact | Mitigation Strategy | Contingency Plan |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **RSK-01** | Kernel Safety | Kernel panic or system lockup caused by faulty pointer dereference in character driver. | Medium | **High** | Test LKM inside isolated QEMU / VirtualBox VM before host loading; use `copy_to_user()` safely. | Use `dmesg` ring buffer analysis and kernel dynamic debugging (`printk`). |
| **RSK-02** | Concurrency | Data race conditions between daemon worker threads accessing process state structures. | Medium | **High** | Enforce strict RAII synchronization using `std::mutex` and `std::scoped_lock`. | Run `valgrind --tool=helgrind` to audit thread safety. |
| **RSK-03** | Rapid Crash Loop | Monitored application continuously crashes, causing CPU saturation during restart loops. | High | **Medium** | Implement exponential backoff timer and maximum restart attempt threshold. | Temporarily mark crashing PID as `FAILED_SUSPENDED` and raise alert. |
| **RSK-04** | Resource Overhead | High file I/O frequency when reading `/proc` files causing CPU overhead > 1.5%. | Low | **Medium** | Optimize file buffer reading; cache static process metadata; adjust tick interval to 1s. | Increase sampling interval dynamically during high system load. |
| **RSK-05** | Socket IPC Fail | Server socket drops connection during remote CLI streaming under network jitter. | Low | **Low** | Implement socket auto-reconnect handling and non-blocking I/O multiplexing (`epoll`). | Buffer un-sent telemetry records in local queue until client reconnects. |

---

## 10. Resource Requirements

### 10.1 Personnel & Roles

* **Lead Systems Developer & Author**: **Harshita Naik** (Responsible for overall platform architecture, C++ daemon development, kernel driver implementation, testing, and documentation).
* **Academic Project Supervisor**: Faculty Advisor, CSE Department, SOA University (Architecture review, milestone verification, and compliance evaluation).

### 10.2 Development & Testing Environments

* **Primary Operating System**: Ubuntu Linux 22.04 LTS / 24.04 LTS (x86_64 64-bit kernel v5.15+ / v6.8+).
* **Virtualization Testbeds**:
  * **QEMU / KVM Virtual Machine**: Isolated kernel testing sandbox to load and debug `sentinel_driver.ko` safely.
  * **Docker Containers**: Controlled environment for simulating process crash scenarios and fault injection testing.

---

## 11. Software and Hardware Requirements

### 11.1 Software Environment Specs

| Tool / Dependency | Minimum Required Version | Recommended Version | Purpose |
| :--- | :--- | :--- | :--- |
| **Operating System** | Linux Kernel 5.4 LTS | Linux Kernel 6.8+ (Ubuntu 24.04 LTS) | Target runtime OS environment. |
| **C++ Compiler** | GCC 11.2 / Clang 13.0 | GCC 13.2+ (`-std=c++20`) | Compiling user-space daemon & client. |
| **C Compiler** | GCC 11.2 | GCC 13.2+ (`-std=c11`) | Compiling Linux Kernel Module. |
| **Build Systems** | GNU Make 4.3, CMake 3.20 | GNU Make 4.4, CMake 3.28+ | Automation of project compilation. |
| **Kernel Headers** | `linux-headers-$(uname -r)` | Matching current kernel release | LKM out-of-tree build requirements. |
| **Debugging Tools** | GDB 12.1, Valgrind 3.18 | GDB 14.1, Valgrind 3.22 | Memory leak and thread audit. |

### 11.2 Hardware Environment Specs

| Hardware Component | Minimum Requirement | Recommended Specification |
| :--- | :--- | :--- |
| **CPU Architecture** | x86_64 or ARM64 (Single Core 1.5 GHz) | x86_64 Multi-Core (4+ Cores @ 2.5 GHz) |
| **System Memory (RAM)** | 2 GB RAM | 8 GB RAM or higher |
| **Disk Storage** | 5 GB available storage | 20 GB SSD storage |
| **Network Adapter** | Standard Ethernet / Wi-Fi Interface | Gigabit Ethernet Interface |

---

## 12. Git Branching & Release Strategy

To maintain codebase stability and support modular feature delivery across stages, SentinelOS enforces a modified **Git Flow** branching model.

```
          [feature/resource-monitor] --------+
                                             |
main   <--------------------------------- develop <--- [feature/process-manager]
  |                                          ^
  +---> [release/v1.0.0-stage2] -------------+
```

### 12.1 Branch Classification

* **`main` Branch**: Production-ready, stable codebase. Commits to `main` are restricted exclusively to tagged release milestones (`v1.0.0-stage1`, `v2.0.0-stage2`, etc.).
* **`develop` Branch**: Main integration branch for active development. Features are merged into `develop` after passing unit tests and static code analysis.
* **`feature/*` Branches**: Feature-specific topic branches created off `develop` (e.g., `feature/resource-monitor`, `feature/kernel-driver`, `feature/self-healing`).
* **`bugfix/*` Branches**: Dedicated branches for resolving defects identified during testing phases (e.g., `bugfix/ioctl-memory-leak`).
* **`release/*` Branches**: Final stage release preparation branches used for tag cutting and documentation consolidation.

### 12.2 Commit Message Convention

Commits must follow the **Conventional Commits** standard to ensure automated changelog generation and clear revision history:

```text
<type>(<scope>): <short summary description>

[optional detailed body explanation]

Examples:
- feat(driver): implement character device open and read file operations
- feat(monitor): add per-core CPU usage parser reading /proc/stat
- fix(recovery): resolve deadlock in SIGCHLD signal handler using std::lock_guard
- docs(stage2): complete Stage 2 SRS and PRD specifications
```

### 12.3 Scope Reduction & Architectural Trade-off Audit

During Stage 2 feasibility analysis, the platform architecture underwent a deliberate scope reduction audit to ensure high reliability, zero system destabilization, and full portability across native Linux and virtualized environments (e.g., WSL2):

1. **Kernel Driver Pivot (eBPF/Kprobes to LKM Character Device)**:
   - *Rationale*: Initial eBPF/Kprobes design introduced heavy kernel version dependencies and required root BPF JIT permissions unsupported in non-custom WSL2 kernels.
   - *Trade-off*: Pivoted to a standard Linux Kernel Module (LKM) character device driver (`/dev/sentinel`) with standard `ioctl` IPC, ensuring deterministic portability across standard Linux kernels.

2. **Dual-Mode Telemetry Architecture (LKM + POSIX Fallback)**:
   - *Rationale*: WSL2 and stripped cloud kernels lack modular device driver loading support by default.
   - *Solution*: Embedded a user-space POSIX filesystem telemetry parser (`/proc/stat`, `/proc/meminfo`, `/proc/diskstats`) as an active fallback when `/dev/sentinel` is unavailable.

---

## 13. Verification & Approval Sign-off

| Role | Name / Designation | Signature / Approval Status | Date |
| :--- | :--- | :--- | :--- |
| **Project Author** | **Harshita Naik** (B.Tech CSE, SOA University) | *Submitted for Review* | September 29, 2026 |
| **Faculty Supervisor** | Department of Computer Science & Engineering | *Pending Stage 2 Review* | Stage 2 Verification |
| **Project Reviewer** | Systems & Embedded Track Evaluator | *Pending Stage 2 Review* | Stage 2 Verification |
