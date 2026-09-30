# SentinelOS: Presentation, Demonstration & Technical Viva Guide

**Project Name:** SentinelOS — Autonomous Linux Health Monitoring, Process Management & Self-Healing Platform  
**Target Audience:** Project Evaluators, University Viva Examiners, Technical Recruiters  
**Author:** Harshita Naik  

---

## 1. Demonstration Flow (Step-by-Step Presentation Script)

Follow this 7-step sequence during live demonstration and evaluation:

1. **Step 1: Clean Compilation & Environment Setup**
   - Demonstrate clean cross-component compilation using `make clean && make all`.
2. **Step 2: Linux Character Device Driver Insertion & Verification**
   - Insert kernel module `sentinel_driver.ko` into the Linux kernel using `sudo insmod`.
   - Inspect kernel ring buffer logs using `dmesg` and verify `/dev/sentinel` node creation.
3. **Step 3: Kernel-User Telemetry Test**
   - Execute `./driver/sentinel_test_app` to show `read()`, `write()`, and `ioctl()` data exchanges between user-space and kernel-space.
4. **Step 4: SentinelOS Monitoring Server Launch**
   - Run `./sentinel_os` to launch the core daemon, initializing `ResourceMonitor`, `KernelMonitor`, `ProcessManager`, `AlertManager`, and `RecoveryManager`.
5. **Step 5: Autonomous Self-Healing Simulation**
   - Watch the `RecoveryManager` detect a simulated `SIGKILL` process crash, trigger self-healing restart, update the recovery dashboard, and log retry limits.
6. **Step 6: Remote TCP Client CLI Interaction**
   - Launch `./sentinel_client 127.0.0.1 9090` in a separate terminal.
   - Query live system metrics: CPU, Memory, Disk, Kernel Release, Process Counts, and System Status.
7. **Step 7: Log Audit & Driver Unload Clean Shutdown**
   - Inspect `logs/sentinel.log` to demonstrate persistent logging.
   - Stop daemon and unload kernel module using `sudo rmmod sentinel_driver`.

---

## 2. Required Screenshots for Presentation / Project Report

Capture these 8 terminal screenshots for inclusion in project slides and documentation:

- **Screenshot 1:** Build execution output (`make clean && make all`).
- **Screenshot 2:** Kernel module loading (`sudo insmod driver/sentinel_driver.ko && dmesg | tail -n 10`).
- **Screenshot 3:** User-kernel space IOCTL execution (`./driver/sentinel_test_app`).
- **Screenshot 4:** SentinelOS main server launch & subsystem online messages (`./sentinel_os`).
- **Screenshot 5:** Self-healing recovery event output (Simulated crash -> Auto-restoration -> Dashboard update).
- **Screenshot 6:** Permanent failure escalation & `CRITICAL` alert triggering.
- **Screenshot 7:** Interactive TCP Client CLI menu responses (`./sentinel_client 127.0.0.1 9090`).
- **Screenshot 8:** Audit log verification (`cat logs/sentinel.log`).

---

## 3. Terminal Commands to Run

```bash
# Phase 1: Build the entire codebase
make clean
make all

# Phase 2: Build & Load Character Device Driver
cd driver
make
sudo insmod sentinel_driver.ko
ls -l /dev/sentinel
dmesg | tail -n 15

# Phase 3: Execute Character Device Test Application
./sentinel_test_app
cd ..

# Phase 4: Launch SentinelOS Monitoring Daemon & Self-Healing Engine
./sentinel_os

# Phase 5: Launch TCP Monitoring Client CLI (Separate Terminal Window)
./sentinel_client 127.0.0.1 9090

# Phase 6: Inspect Persistent Log File
cat logs/sentinel.log

# Phase 7: Unload Device Driver
cd driver
sudo rmmod sentinel_driver
dmesg | tail -n 5
```

---

## 4. Sample Outputs

### 4.1 Server Daemon & Self-Healing Engine Output (`./sentinel_os`)
```text
==========================================================================================
            SentinelOS: Autonomous Health Monitoring & Self-Healing Platform             
==========================================================================================
[2026-09-30 14:54:55] [INFO] [RecoveryEngine] - Self-Healing Recovery Manager initialized.

[Subsystems Online] ResourceMonitor, KernelMonitor, ProcessManager, AlertManager, RecoveryManager.
[2026-09-30 14:54:55] [INFO] [RecoveryEngine] - Registered process 'WorkerDaemon' [Policy: IMMEDIATE, Max Retries: 3]
[2026-09-30 14:54:55] [INFO] [RecoveryEngine] - Registered process 'DatabaseBridge' [Policy: DELAYED, Max Retries: 2]

[Step 1] Starting supervised processes...
[2026-09-30 14:54:55] [INFO] [RecoveryEngine] - Spawned process 'WorkerDaemon' with PID: 4001
[2026-09-30 14:54:55] [INFO] [RecoveryEngine] - Spawned process 'DatabaseBridge' with PID: 4002

==========================================================================================
                 SENTINEL OS: SELF-HEALING RECOVERY DASHBOARD                            
==========================================================================================
 PROCESS NAME       | PID     | STATUS           | RECOVERY COUNT | POLICY     | LAST RECOVERY TIME
-------------------+---------+------------------+---------------+------------+--------------------
 DatabaseBridge     | 4002    | RUNNING          | 0 / 2          | DELAYED    | N/A
 WorkerDaemon       | 4001    | RUNNING          | 0 / 3          | IMMEDIATE  | N/A
==========================================================================================

[TEST CASE 1] Simulating Process Crash for 'WorkerDaemon' (SIGKILL)...
[2026-09-30 14:54:56] [ERROR] [RecoveryEngine] - Process Crash Detected! Name: 'WorkerDaemon' | PID: 4001
[2026-09-30 14:54:56] [WARNING] [AlertManager] - WARNING: Process failure detected for 'WorkerDaemon' (PID: 4001). Self-healing triggered.
[2026-09-30 14:54:56] [INFO] [RecoveryEngine] - Self-Healing Attempt 1/3 for process 'WorkerDaemon'...
[2026-09-30 14:54:56] [INFO] [RecoveryEngine] - SUCCESS: Process 'WorkerDaemon' automatically restored! New PID: 4003

==========================================================================================
                 SENTINEL OS: SELF-HEALING RECOVERY DASHBOARD                            
==========================================================================================
 PROCESS NAME       | PID     | STATUS           | RECOVERY COUNT | POLICY     | LAST RECOVERY TIME
-------------------+---------+------------------+---------------+------------+--------------------
 DatabaseBridge     | 4002    | RUNNING          | 0 / 2          | DELAYED    | N/A
 WorkerDaemon       | 4003    | RUNNING          | 1 / 3          | IMMEDIATE  | 2026-09-30 14:54:56
==========================================================================================
```

### 4.2 User-Kernel Driver Test Output (`./driver/sentinel_test_app`)
```text
=================================================================
   SentinelOS User-Space Kernel Device Communication Test        
=================================================================
[1] Opening character device /dev/sentinel...
[SUCCESS] Opened /dev/sentinel successfully (File Descriptor: 3)

[2] Reading telemetry banner from kernel device...
[DRIVER READ RESULT] Received 68 bytes:
[SentinelOS Kernel Subsystem Active - Major: 236, Minor: 0]

[3] Sending command message to kernel device...
    Write Payload: 'SENTINEL_CMD: ENABLE_PROCESS_WATCHDOG_MODE'
[SUCCESS] Wrote 42 bytes to kernel device.

[4] Querying Driver Telemetry Status over IOCTL...
    [IOCTL Telemetry Result]
    - Driver Version : 1.0.0
    - Major Number   : 236
    - Minor Number   : 0
    - Total Reads    : 1
    - Total Writes   : 1
    - Buffer Data Len: 42 bytes

[5] Querying Kernel Memory Telemetry over IOCTL...
    [Kernel Memory Telemetry]
    - Total Kernel RAM : 15982 MB (16365977 KB)
    - Free Kernel RAM  : 7412 MB (7590200 KB)
    - Page Size        : 4096 bytes

[6] Closing device handle...
[SUCCESS] Closed /dev/sentinel. Communication test completed successfully.
=================================================================
```

---

## 5. Technical Viva Questions & Answers (Top 15)

### Q1: What is SentinelOS and what problem does it solve?
**Answer:** SentinelOS is an autonomous Linux health monitoring and self-healing platform. It eliminates manual intervention by continuously capturing CPU, memory, disk, and kernel telemetry via `/proc`, providing remote TCP server monitoring, and automatically restarting crashed system daemons using POSIX process management APIs.

### Q2: How does `ResourceMonitor` calculate CPU usage without external libraries?
**Answer:** It reads `/proc/stat` to extract raw CPU tick counters (user, nice, system, idle, iowait, irq, softirq, steal). By computing deltas between consecutive reads:
$$\text{CPU Usage \%} = \left(1.0 - \frac{\Delta \text{idle\_ticks}}{\Delta \text{total\_ticks}}\right) \times 100.0$$

### Q3: How does SentinelOS retrieve Linux Kernel information?
**Answer:** It uses the POSIX `uname()` system call to query system release, hostname, and machine architecture, and parses `/proc/version`, `/proc/cpuinfo`, `/proc/uptime`, and `/proc/meminfo` for hardware details.

### Q4: Explain the difference between `copy_to_user()` and `copy_from_user()` in Linux device drivers.
**Answer:** Kernel space and user space reside in distinct virtual address spaces. Direct pointer dereferencing across spaces causes kernel panics or security flaws. `copy_to_user()` safely transfers data from kernel memory buffers to user-space pointers, and `copy_from_user()` safely reads data from user-space pointers into kernel memory, returning uncopied byte counts upon faults.

### Q5: How is `/dev/sentinel` character device registered with the kernel?
**Answer:** 
1. `alloc_chrdev_region()` dynamically allocates a major/minor device number range.
2. `cdev_init()` binds character device file operations (`struct file_operations fops`).
3. `cdev_add()` registers the device with VFS.
4. `class_create()` and `device_create()` generate sysfs entries so udev automatically creates `/dev/sentinel`.

### Q6: What is an IOCTL and why is it used in SentinelOS?
**Answer:** `ioctl()` (Input/Output Control) allows user-space applications to issue structured control commands and query binary telemetry structures from device drivers that don't fit standard stream `read()` or `write()` calls. SentinelOS uses IOCTLs to query driver version, read counter statistics (`SENTINEL_IOCTL_GET_STATUS`), and inspect kernel RAM usage (`SENTINEL_IOCTL_GET_KERN_MEM`).

### Q7: Why did you use `mutex_lock_interruptible()` instead of `mutex_trylock()` in the character driver?
**Answer:** `mutex_trylock()` fails immediately with `-EBUSY` if another process holds the driver lock. `mutex_lock_interruptible()` puts the calling process to sleep cleanly until the lock becomes available, while allowing OS signal interruptions (returning `-ERESTARTSYS`).

### Q8: How does `RecoveryManager` detect a process failure?
**Answer:** `RecoveryManager` executes non-blocking `waitpid(proc.pid, &wstatus, WNOHANG)`. If `waitpid()` returns the child PID, it checks `WIFEXITED(wstatus)` or `WIFSIGNALED(wstatus)` to determine if the process exited unexpectedly or was killed by a signal (e.g., `SIGKILL`). It also uses `kill(pid, 0)` to verify process existence.

### Q9: How does process spawning work in `RecoveryManager::spawn_process()`?
**Answer:** It uses POSIX `fork()` to create a child process duplicate. The child process calls `execvp(binary_path, argv)` to replace its image with the target executable. The parent process receives the child PID from `fork()` and stores it in the supervision table.

### Q10: What is a Zombie process and how does SentinelOS prevent zombie leaks?
**Answer:** A Zombie process (`'Z'`) is a terminated process whose entry remains in the kernel process table because its parent has not read its exit code via `wait()` / `waitpid()`. `RecoveryManager` invokes `waitpid(..., WNOHANG)` during health checks to reap terminated children and prevent table saturation.

### Q11: How is thread safety enforced across SentinelOS C++ components?
**Answer:** `Logger` uses `std::mutex` with `std::lock_guard` to synchronize concurrent log writes. `RecoveryManager` uses `m_mutex` to protect the `unordered_map` supervision registry. `Server` uses `m_monitor_mutex` when retrieving telemetry for client connections.

### Q12: Explain the purpose of `SO_REUSEADDR` in `Server.cpp`.
**Answer:** When a server socket closes, it enters a `TIME_WAIT` state. Setting `SO_REUSEADDR` via `setsockopt()` allows the server to immediately rebind to TCP port 9090 upon restart without raising an "Address already in use" (`EADDRINUSE`) error.

### Q13: How does the client-server protocol work in SentinelOS?
**Answer:** The client opens an IPv4 TCP stream socket to port 9090 and sends plaintext ASCII command strings (`GET_CPU`, `GET_MEMORY`, `GET_DISK`, `GET_KERNEL`, `GET_PROCESSES`, `GET_SYSTEM_STATUS`). The multithreaded server processes the string under lock and returns a formatted key-value response stream.

### Q14: How does SentinelOS handle recovery policy retries?
**Answer:** Each monitored process has a `max_retries` counter and policy (`IMMEDIATE` or `DELAYED`). If retries are below threshold, it increments `retry_count` and restarts the binary (pausing for `cooldown_seconds` under `DELAYED`). If retries equal `max_retries`, it transitions the status to `FAILED_PERMANENT` and issues a `CRITICAL` alert.

### Q15: Why is `ThreadCompat.h` included in the project?
**Answer:** Standard C++ `<thread>` and `<mutex>` implementations vary across GCC, Clang, and MinGW compilers on Windows/Linux. `ThreadCompat.h` provides cross-platform abstractions so the project compiles seamlessly across Native Linux GCC and Windows MinGW toolchains.

---

## 6. Common Debugging & Troubleshooting Issues

| Issue | Root Cause | Solution |
| :--- | :--- | :--- |
| **`Permission denied` accessing `/dev/sentinel`** | Default character device creation sets root-only permissions (`0600`). | Set `sentinel_class->devnode` callback to `0666` in driver, or run `sudo chmod 666 /dev/sentinel`. |
| **`Address already in use` error on port 9090** | Previous server process terminated without releasing socket, or port in `TIME_WAIT`. | Ensure `SO_REUSEADDR` is set in `Server.cpp`, or kill existing process using `fuser -k 9090/tcp`. |
| **Zombie processes accumulating in process table** | Spawned child process exited but parent did not call `waitpid()`. | Ensure `RecoveryManager::check_and_heal_process()` invokes `waitpid(pid, &wstatus, WNOHANG)`. |
| **Kernel driver build failure (`KDIR` not found)** | Linux kernel build headers missing for active kernel release. | Install headers via `sudo apt install linux-headers-$(uname -r)`. |
| **`inet_pton` undeclared in Windows MinGW compilation** | Windows Sockets version macro `_WIN32_WINNT` not defined before header inclusion. | Pass `-D_WIN32_WINNT=0x0600` in compiler flags and include `<winsock2.h>` / `<ws2tcpip.h>`. |

---

## 7. Key Project Achievements

- **Autonomous Self-Healing:** Built an engine capable of recovering failed daemons via POSIX `fork`/`execvp` and configurable recovery policies.
- **Kernel-Space Telemetry Driver:** Developed a custom Linux Character Device Driver (`/dev/sentinel`) with IOCTL support and safe user-kernel memory isolation.
- **Real-Time Telemetry Pipeline:** Implemented non-blocking `/proc` hardware and resource monitoring without external dependencies.
- **Multithreaded Remote Architecture:** Built a TCP Server & CLI Client architecture supporting system administration queries over TCP port 9090.
- **Production-Grade Thread Safety:** Enforced synchronization using mutex guards across background monitoring loops and socket handler threads.

---

## 8. Future Scope & Enhancements

- **eBPF (Extended Berkeley Packet Filter) Integration:** Replace `/proc` directory polling with kernel eBPF probes for zero-overhead event tracing.
- **Web Dashboard & WebSockets:** Develop a modern React/TypeScript frontend with real-time WebSocket telemetry updates.
- **ML-Driven Anomaly Prediction:** Implement machine learning algorithms (e.g., Isolation Forests) to predict process failures prior to crashes.
- **Prometheus & Grafana Exporter:** Add a `/metrics` HTTP endpoint to export SentinelOS telemetry directly to Prometheus monitoring stacks.
- **TLS/SSL Socket Encryption:** Secure client-server TCP communication using OpenSSL certificate-based encryption.

---

## 9. Resume Project Description

**SentinelOS — Autonomous Linux Health Monitoring & Self-Healing Platform**  
*C++, Linux System Programming, Linux Device Drivers, POSIX APIs, Multithreading, TCP Sockets*  
- Developed an autonomous Linux health monitoring platform in C++ to track CPU, memory, disk, and process telemetry using `/proc` file interfaces.
- Engineered a Self-Healing Recovery Engine using POSIX `fork()`, `execvp()`, and `waitpid()` system calls to detect process crashes and execute automated restarts.
- Developed a Linux Character Device Driver (`/dev/sentinel`) implementing `read()`, `write()`, and `ioctl()` handlers for user-kernel space memory copy.
- Built a multithreaded TCP Server and CLI Client operating over TCP port 9090 for remote telemetry inspection.

---

## 10. LinkedIn Project Showcase Post

🚀 **Excited to share my latest Linux System Programming & Kernel project: SentinelOS!** 🐧⚡

SentinelOS is an **Autonomous Linux Health Monitoring, Process Management & Self-Healing Platform** built from scratch in C++ and Linux System Programming primitives!

💡 **Key Highlights:**
🔹 **Self-Healing Engine:** Automatically detects crashed system services using POSIX `fork()`, `execvp()`, and `waitpid()`, restarting failed processes according to configurable recovery policies (`IMMEDIATE` / `DELAYED`).  
🔹 **Linux Character Device Driver:** Custom kernel module (`/dev/sentinel`) enabling secure user-kernel space telemetry exchange via POSIX file operations and custom `ioctl()` commands.  
🔹 **Real-Time Telemetry:** Non-blocking CPU, RAM, Disk, and Process state monitoring directly from Linux `/proc` filesystem interfaces.  
🔹 **Remote Monitoring TCP Architecture:** Multithreaded TCP Server & Client for remote system administration over port 9090.  

#Linux #Cpp #SystemProgramming #LinuxKernel #DeviceDrivers #OperatingSystems #SoftwareEngineering #ComputerScience
