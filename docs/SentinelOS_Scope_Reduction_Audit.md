# SentinelOS: Academic Scope Reduction & Software Architecture Audit

**Author:** Senior Linux System Programming Professor & Software Architect  
**Document Type:** Project Scope Reduction & Optimization Audit  
**Target Platform:** Linux Only  

---

## 1. Executive Summary & Audit Rationale

The objective of this audit is to streamline **SentinelOS** by eliminating non-essential, high-complexity modules while retaining **100% of required academic features**.

By removing auxiliary systems (Self-Healing Engine, Alert Manager, Singleton Logger, Windows Abstraction Layers), the codebase size is reduced by **~65%**, eliminating race conditions, deadlocks, and build conflicts, making it feasible for a single student to study and explain during a viva examination.

---

## 2. File-by-File Categorization & Evaluation Justification

| File Path | Category | Rationale & Evaluation Impact |
| :--- | :---: | :--- |
| `include/ResourceMonitor.h` <br> `src/ResourceMonitor.cpp` | **KEEP** | **Core Module 1 Requirement:** Computes CPU %, RAM usage, and Disk capacity. Essential for system telemetry evaluation. |
| `include/ProcessManager.h` <br> `src/ProcessManager.cpp` | **KEEP** | **Core Module 2 Requirement:** Scans `/proc` to retrieve active PIDs, binary names, and process states (`'R'`, `'S'`, `'Z'`). |
| `include/KernelMonitor.h` <br> `src/KernelMonitor.cpp` | **KEEP** | **Core Module 3 Requirement:** Uses `uname()` and `/proc/version`, `/proc/uptime` to extract OS release and hardware specs. |
| `include/Server.h` <br> `src/Server.cpp` | **KEEP** | **Core Module 4 Requirement:** Multithreaded TCP socket server listening on port 9090, fulfilling Client-Server architecture. |
| `client/Client.h` <br> `client/Client.cpp` <br> `client/client_main.cpp` | **KEEP** | **Core Module 4 Requirement:** Remote CLI TCP client querying telemetry from server over sockets. |
| `driver/sentinel_driver.c` | **KEEP** | **Core Module 5 Requirement:** Linux Character Device Driver implementing `/dev/sentinel` with `open()`, `read()`, `write()`. |
| `driver/sentinel_test_app.c` | **KEEP** | Demonstrates user-space interaction with character device `/dev/sentinel` for viva evaluators. |
| `src/main.cpp` | **KEEP** | Server daemon entry point. Initializes telemetry objects and starts TCP listener. |
| `include/RecoveryManager.h` <br> `src/RecoveryManager.cpp` | **REMOVE** | **High Complexity / Low Mark Return:** Adds 500+ lines of POSIX `fork`/`execvp`/`waitpid` self-healing code, retry loops, and sleep-under-lock bugs. Not in mandatory 5 modules. |
| `include/AlertManager.h` <br> `src/AlertManager.cpp` | **REMOVE** | **Redundant Overhead:** Duplicate `if/else` threshold checks. Replaced by direct console output. |
| `include/Logger.h` <br> `src/Logger.cpp` | **REMOVE** | **Unnecessary Abstraction:** Singleton pattern with mutex locks, timestamp formatters, and file I/O streams. Direct `std::cout` is simpler and transparent. |
| `include/App.h` <br> `src/App.cpp` | **REMOVE** | **Unnecessary Wrapper:** Intermediate class between `main.cpp` and telemetry modules. Obscures program flow. |
| `include/ThreadCompat.h` | **REMOVE** | **Violates Linux-Only Focus:** Windows/MinGW thread/mutex compatibility header. Standard `<thread>` and `<mutex>` work natively on Linux. |
| `driver/sentinel_ioctl.h` | **REMOVE** | **Optional Complexity:** Requirement specifies `open()`, `read()`, `write()`. Removing IOCTL reduces kernel driver to ~70 lines of clean C. |

---

## 3. Safe Code & Module Removal List

The following components add code complexity without improving academic evaluation marks:

1. **Recovery Engine (`RecoveryManager.cpp` / `.h`)**:
   - *Lines Saved:* ~530 lines.
   - *Reason:* Complex child process supervision, signal handling, and retry policy management. Evaluators focus on baseline Linux System Programming, not custom self-healing framework mechanics.
2. **Singleton Logging System (`Logger.cpp` / `.h`)**:
   - *Lines Saved:* ~180 lines.
   - *Reason:* Replaced by direct `std::cout` and `std::cerr` streams.
3. **Application Wrapper Layer (`App.cpp` / `.h`)**:
   - *Lines Saved:* ~100 lines.
   - *Reason:* Eliminates out-of-sync method calls and signature mismatch build bugs.
4. **Cross-Platform Thread Compatibility (`ThreadCompat.h`)**:
   - *Lines Saved:* ~120 lines.
   - *Reason:* Linux-native GCC handles `#include <thread>` and `#include <mutex>` without wrappers.
5. **Driver IOCTL Subsystem (`sentinel_ioctl.h`)**:
   - *Lines Saved:* ~50 lines.
   - *Reason:* Focuses kernel module strictly on `open()`, `read()`, and `write()`.

---

## 4. Final Minimal Folder Structure

```text
SentinelOS/
├── include/
│   ├── ResourceMonitor.h   # Module 1: CPU, RAM, Disk Monitor Header
│   ├── ProcessManager.h    # Module 2: Process Scanner Header
│   ├── KernelMonitor.h     # Module 3: Kernel Information Header
│   ├── Server.h            # Module 4: TCP Server Header
│   └── Client.h            # Module 4: TCP Client Header
│
├── src/
│   ├── ResourceMonitor.cpp # Module 1 Implementation
│   ├── ProcessManager.cpp  # Module 2 Implementation
│   ├── KernelMonitor.cpp   # Module 3 Implementation
│   ├── Server.cpp          # Module 4 Server Implementation
│   ├── Client.cpp          # Module 4 Client Implementation
│   └── main.cpp            # Application Entry Point
│
├── client/
│   └── client_main.cpp     # Client CLI Entry Point
│
├── driver/
│   ├── sentinel_driver.c   # Module 5: Linux Character Device Driver
│   ├── sentinel_test_app.c # User-space Character Device Test App
│   └── Makefile            # Kernel Module Kbuild Makefile
│
├── Makefile                # Root Application Makefile
└── README.md               # Documentation & Execution Guide
```

---

## 5. Final Dependency Diagram

```mermaid
graph TD
    subgraph Client Application Target
        ClientMain["client_main.cpp"] --> ClientClass["Client (Client.cpp)"]
    end

    subgraph Server Daemon Target
        ServerMain["main.cpp"] --> ServerClass["Server (Server.cpp)"]
        ServerClass --> RM["ResourceMonitor"]
        ServerClass --> PM["ProcessManager"]
        ServerClass --> KM["KernelMonitor"]
    end

    subgraph Linux Kernel Character Device Target
        TestApp["sentinel_test_app.c"] -->|open / read / write| DevNode["/dev/sentinel"]
        DevNode --> Driver["sentinel_driver.c (GPL Module)"]
    end

    ClientClass <-->|TCP Socket (Port 9090)| ServerClass
```

---

## 6. Implementation & Cleanup Checklist

- [ ] Delete `include/RecoveryManager.h` and `src/RecoveryManager.cpp`.
- [ ] Delete `include/AlertManager.h` and `src/AlertManager.cpp`.
- [ ] Delete `include/Logger.h` and `src/Logger.cpp`.
- [ ] Delete `include/App.h` and `src/App.cpp`.
- [ ] Delete `include/ThreadCompat.h`.
- [ ] Delete `driver/sentinel_ioctl.h`.
- [ ] Update `src/main.cpp` to initialize telemetry modules and start `Server`.
- [ ] Update root `Makefile` to compile simplified `sentinel_os` and `sentinel_client`.
- [ ] Update `driver/sentinel_driver.c` to contain minimal `open`, `read`, `write`, `release` handlers.

---

## 7. Viva Preparation Checklist (Simplified Project Scope)

Be prepared to answer these core questions during viva examination:

1. **Linux Telemetry (`/proc` filesystem)**:
   - *Question:* How does `ResourceMonitor` read CPU usage?
   - *Answer:* Reads `/proc/stat` first line (`cpu ...`), calculates deltas between consecutive idle ticks vs total ticks: $\text{CPU\%} = (1 - \Delta \text{idle} / \Delta \text{total}) \times 100$.
2. **Process Management**:
   - *Question:* How does `ProcessManager` scan active processes?
   - *Answer:* Opens `/proc` using `opendir()`, iterates numerical folders (`isdigit`), and parses `/proc/[pid]/stat` for binary name and process state (`'R'`, `'S'`, `'Z'`).
3. **Kernel Telemetry**:
   - *Question:* How do you get system uptime and kernel release?
   - *Answer:* POSIX `uname(&buf)` for release and machine architecture; `/proc/uptime` for uptime seconds.
4. **Socket Programming**:
   - *Question:* Explain your server socket workflow.
   - *Answer:* `socket(AF_INET, SOCK_STREAM, 0)` $\rightarrow$ `setsockopt(SO_REUSEADDR)` $\rightarrow$ `bind(9090)` $\rightarrow$ `listen(5)` $\rightarrow$ `accept()`. Spawns worker thread to process text commands (`GET_CPU`, `GET_MEMORY`, etc.).
5. **Character Device Driver**:
   - *Question:* Explain character device registration in `sentinel_driver.c`.
   - *Answer:* `alloc_chrdev_region()` allocates major number; `cdev_init()` & `cdev_add()` bind `file_operations` (`open`, `read`, `write`); `class_create()` & `device_create()` generate `/dev/sentinel`.
6. **Kernel Space vs User Space Data Transfer**:
   - *Question:* Why can't the driver use standard `memcpy()`?
   - *Answer:* Kernel and User space operate in separate virtual address spaces. Drivers must use `copy_to_user()` for reads and `copy_from_user()` for writes to handle page faults safely.
