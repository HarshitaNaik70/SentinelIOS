# SentinelOS: Simplified Academic Architecture & Implementation Blueprint

**Project Name:** SentinelOS — Modular Linux System Telemetry & Character Device Driver Platform  
**Target Platform:** Linux Only (Ubuntu 22.04 / 24.04 LTS, Kernel 6.x)  
**Language:** C++17 (User-Space Application) & C (Kernel Module)  
**Role:** Senior Linux System Programming & Device Driver Architect Blueprint  

---

## 1. Simplified Final Architecture

The simplified SentinelOS architecture decouples into three distinct layers:
1. **Telemetry & System Layer:** Non-blocking `/proc` and POSIX query modules (`ResourceMonitor`, `ProcessManager`, `KernelMonitor`).
2. **Network Inter-Process Communication Layer:** Multithreaded TCP Server (`Server`) exposing text-based telemetry APIs to a remote CLI Client (`Client`).
3. **Kernel Device Interface Layer:** Linux Character Device Driver (`/dev/sentinel`) demonstrating VFS `open()`, `read()`, `write()`, and dynamic chrdev registration.

```mermaid
graph TD
    subgraph User Space Remote CLI
        Client["TCP Client (Client.cpp)"]
    end

    subgraph User Space SentinelOS Daemon
        Server["TCP Server (Server.cpp)"]
        RM["ResourceMonitor (ResourceMonitor.cpp)"]
        PM["ProcessManager (ProcessManager.cpp)"]
        KM["KernelMonitor (KernelMonitor.cpp)"]
        TestApp["Driver Test Utility (sentinel_test.c)"]
        
        Server -->|Queries Telemetry| RM
        Server -->|Queries Telemetry| PM
        Server -->|Queries Telemetry| KM
    end

    subgraph Linux Kernel Space
        VFS["VFS /dev/sentinel Node"]
        Driver["Character Device Driver (sentinel_driver.c)"]
        ProcFS["Linux /proc Filesystem"]

        TestApp -->|open / read / write| VFS
        VFS --> Driver
        RM -->|Reads /proc/stat, /proc/meminfo| ProcFS
        PM -->|Scans /proc/[pid]/stat| ProcFS
        KM -->|Reads /proc/version, uname()| ProcFS
    end

    Client <-->|TCP Port 9090| Server
```

---

## 2. Minimal Folder Structure

```text
SentinelOS/
├── include/
│   ├── ResourceMonitor.h   # CPU, RAM, Disk header
│   ├── ProcessManager.h    # PID, Name, State process scanner header
│   ├── KernelMonitor.h     # Kernel release, uptime, CPU hardware header
│   ├── Server.h            # Multithreaded TCP Server header
│   └── Client.h            # Remote CLI Client header
│
├── src/
│   ├── ResourceMonitor.cpp # CPU, RAM, Disk implementation
│   ├── ProcessManager.cpp  # /proc PID scanner implementation
│   ├── KernelMonitor.cpp   # uname() and kernel info implementation
│   ├── Server.cpp          # TCP Socket bind/listen/accept implementation
│   ├── Client.cpp          # TCP Socket connect/send/recv implementation
│   └── main.cpp            # Server daemon entry point
│
├── driver/
│   ├── sentinel_driver.c   # Linux Character Device Driver (open, read, write)
│   ├── sentinel_test.c     # User-space device test app
│   └── Makefile            # Kbuild kernel module Makefile
│
├── docs/
│   └── Architecture_Doc.md # Architectural design document
│
├── Makefile                # Root user-space application Makefile
└── README.md               # GitHub project documentation & execution guide
```

---

## 3. Required C++ Classes

1. **`ResourceMonitor`**: Encapsulates CPU utilization calculation via `/proc/stat`, Memory parsing via `/proc/meminfo`, and Disk capacity calculation via `statvfs()`.
2. **`ProcessManager`**: Discovers active PIDs in `/proc`, parses process binary name and state (`'R'`, `'S'`, `'Z'`) from `/proc/[pid]/stat`.
3. **`KernelMonitor`**: Extracts kernel release, machine architecture using `uname()`, CPU model name from `/proc/cpuinfo`, and system uptime from `/proc/uptime`.
4. **`Server`**: Encapsulates IPv4 TCP socket initialization (`socket`, `bind`, `listen`, `accept`), running a worker thread loop to serve incoming client telemetry queries on port 9090.
5. **`Client`**: Manages outgoing TCP socket connections (`socket`, `connect`, `send`, `recv`), providing an interactive CLI menu for telemetry inspection.

---

## 4. File-by-File Breakdown: Source Files (`src/` & `driver/`)

### 1. `src/ResourceMonitor.cpp`
- **Why it exists:** Computes real-time CPU, RAM, and Disk metrics.
- **Concepts demonstrated:** File I/O parsing (`std::ifstream`), tick delta math, POSIX `statvfs()` system call.
- **Why important for evaluation:** Proves understanding of Linux system metrics collection without third-party libraries.

### 2. `src/ProcessManager.cpp`
- **Why it exists:** Scans `/proc` to enumerate running processes and parse process states.
- **Concepts demonstrated:** Directory traversing (`opendir`, `readdir`), numerical PID filtering, POSIX string parsing.
- **Why important for evaluation:** Demonstrates mastery of Linux process structures and `/proc` pseudo-filesystem representation.

### 3. `src/KernelMonitor.cpp`
- **Why it exists:** Collects low-level system host and hardware information.
- **Concepts demonstrated:** POSIX `uname()` system call, string stream parsing.
- **Why important for evaluation:** Highlights interface with kernel data structures and hardware platform telemetry.

### 4. `src/Server.cpp`
- **Why it exists:** Listens on TCP port 9090 and serves incoming monitoring requests.
- **Concepts demonstrated:** Socket programming (`socket`, `bind`, `listen`, `accept`, `recv`, `send`), `SO_REUSEADDR`, multi-threading (`std::thread`).
- **Why important for evaluation:** Satisfies requirement for Client-Server Architecture and POSIX networking.

### 5. `src/Client.cpp`
- **Why it exists:** Connects remotely to the SentinelOS server and displays interactive monitoring data.
- **Concepts demonstrated:** Client TCP sockets (`connect`), buffer management, CLI menu loop.
- **Why important for evaluation:** Proves full end-to-end network communication between client and server binaries.

### 6. `src/main.cpp`
- **Why it exists:** Application entry point that initializes monitoring modules and starts the TCP Server daemon.
- **Concepts demonstrated:** Modular initialization, lifecycle management.
- **Why important for evaluation:** Demonstrates clean software design pattern and software architecture.

### 7. `driver/sentinel_driver.c`
- **Why it exists:** Custom Linux Kernel Module implementing character device `/dev/sentinel`.
- **Concepts demonstrated:** Kernel module entry/exit (`module_init`, `module_exit`), dynamic chrdev region allocation (`alloc_chrdev_region`), file operations (`fops`), `copy_to_user()`, `copy_from_user()`, kernel mutex locking (`mutex_lock_interruptible`).
- **Why important for evaluation:** Crucial kernel-level component satisfying the Linux Device Driver academic requirement.

### 8. `driver/sentinel_test.c`
- **Why it exists:** User-space C verification program for testing character device interactions.
- **Concepts demonstrated:** POSIX system calls (`open`, `read`, `write`, `close`).
- **Why important for evaluation:** Proves functional operation of the character device driver from user-space.

---

## 5. File-by-File Breakdown: Header Files (`include/`)

### 1. `include/ResourceMonitor.h`
- **Why it exists:** Defines `CpuTicks`, `MemoryInfo`, `DiskInfo` structs and `ResourceMonitor` class interface.
- **Concepts demonstrated:** Data abstraction, encapsulated telemetry interfaces.
- **Why important for evaluation:** Enforces modular contract between monitoring implementations.

### 2. `include/ProcessManager.h`
- **Why it exists:** Defines `ProcessInfo` struct and `ProcessManager` class interface.
- **Concepts demonstrated:** Object-oriented process model.
- **Why important for evaluation:** Provides clean separation of process management logic.

### 3. `include/KernelMonitor.h`
- **Why it exists:** Defines `KernelVersionInfo`, `CpuHardwareInfo`, `SystemInfo` structs and `KernelMonitor` class interface.
- **Concepts demonstrated:** Data structure design for hardware telemetry.
- **Why important for evaluation:** Clear specification of kernel metadata interfaces.

### 4. `include/Server.h`
- **Why it exists:** Declares `Server` class for managing network daemon state.
- **Concepts demonstrated:** Network server abstraction.
- **Why important for evaluation:** Demonstrates structured concurrent network design.

### 5. `include/Client.h`
- **Why it exists:** Declares `Client` class interface for remote server interaction.
- **Concepts demonstrated:** Network client abstraction.
- **Why important for evaluation:** Clean separation of user interface logic from underlying socket communication.

---

## 6. Device Driver Files (`driver/`)

### `driver/sentinel_driver.c`
```c
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SentinelOS Core Team");
MODULE_DESCRIPTION("SentinelOS Linux Character Device Driver");
MODULE_VERSION("1.0.0");

#define DEVICE_NAME "sentinel"
#define CLASS_NAME "sentinel_class"
#define BUFFER_SIZE 1024

static int major_number;
static struct class* sentinel_class = NULL;
static struct device* sentinel_device = NULL;
static struct cdev sentinel_cdev;
static char kernel_buffer[BUFFER_SIZE];
static size_t buffer_data_len = 0;
static DEFINE_MUTEX(sentinel_mutex);

static int sentinel_open(struct inode *inodep, struct file *filep) {
    pr_info("sentinel_driver: Device opened by PID %d\n", current->pid);
    return 0;
}

static int sentinel_release(struct inode *inodep, struct file *filep) {
    pr_info("sentinel_driver: Device closed by PID %d\n", current->pid);
    return 0;
}

static ssize_t sentinel_read(struct file *filep, char __user *buffer, size_t len, loff_t *offset) {
    size_t bytes_to_read;
    if (mutex_lock_interruptible(&sentinel_mutex)) return -ERESTARTSYS;

    if (*offset >= buffer_data_len) {
        mutex_unlock(&sentinel_mutex);
        return 0;
    }

    bytes_to_read = min(len, (size_t)(buffer_data_len - *offset));
    if (copy_to_user(buffer, kernel_buffer + *offset, bytes_to_read) != 0) {
        mutex_unlock(&sentinel_mutex);
        return -EFAULT;
    }

    *offset += bytes_to_read;
    mutex_unlock(&sentinel_mutex);
    return bytes_to_read;
}

static ssize_t sentinel_write(struct file *filep, const char __user *buffer, size_t len, loff_t *offset) {
    size_t bytes_to_copy;
    if (mutex_lock_interruptible(&sentinel_mutex)) return -ERESTARTSYS;

    bytes_to_copy = min(len, (size_t)(BUFFER_SIZE - 1));
    memset(kernel_buffer, 0, BUFFER_SIZE);

    if (copy_from_user(kernel_buffer, buffer, bytes_to_copy) != 0) {
        mutex_unlock(&sentinel_mutex);
        return -EFAULT;
    }

    kernel_buffer[bytes_to_copy] = '\0';
    buffer_data_len = bytes_to_copy;
    mutex_unlock(&sentinel_mutex);
    return bytes_to_copy;
}

static char *sentinel_devnode(const struct device *dev, umode_t *mode) {
    if (mode) *mode = 0666;
    return NULL;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = sentinel_open,
    .release = sentinel_release,
    .read = sentinel_read,
    .write = sentinel_write,
};

static int __init sentinel_init(void) {
    dev_t dev;
    if (alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME) < 0) return -1;
    major_number = MAJOR(dev);

    cdev_init(&sentinel_cdev, &fops);
    cdev_add(&sentinel_cdev, dev, 1);

    sentinel_class = class_create(CLASS_NAME);
    if (IS_ERR(sentinel_class)) {
        unregister_chrdev_region(dev, 1);
        return PTR_ERR(sentinel_class);
    }
    sentinel_class->devnode = sentinel_devnode;

    sentinel_device = device_create(sentinel_class, NULL, dev, NULL, DEVICE_NAME);
    snprintf(kernel_buffer, BUFFER_SIZE, "[SentinelOS Kernel Engine Ready]\n");
    buffer_data_len = strlen(kernel_buffer);
    return 0;
}

static void __exit sentinel_exit(void) {
    dev_t dev = MKDEV(major_number, 0);
    device_destroy(sentinel_class, dev);
    class_destroy(sentinel_class);
    cdev_del(&sentinel_cdev);
    unregister_chrdev_region(dev, 1);
}

module_init(sentinel_init);
module_exit(sentinel_exit);
```

### `driver/Makefile`
```makefile
obj-m += sentinel_driver.o

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

.PHONY: default clean
```

---

## 7. Development Order (5-Day Implementation Roadmap)

- **Day 1: Telemetry Core Subsystems**
  - Implement `ResourceMonitor`, `ProcessManager`, and `KernelMonitor`. Test parsing of `/proc/stat`, `/proc/meminfo`, `/proc/uptime`, and `/proc/[pid]/stat`.
- **Day 2: Client-Server Network Layer**
  - Implement `Server` and `Client` socket handling. Verify command request/response loop over TCP port 9090.
- **Day 3: Character Device Driver**
  - Implement `sentinel_driver.c` and `sentinel_test.c`. Compile module, test `insmod`, `/dev/sentinel` node access, `read()`, `write()`, and `rmmod`.
- **Day 4: Integration & Build Automation**
  - Create root `Makefile`. Integrate telemetry modules into `main.cpp` server daemon. Conduct end-to-end testing.
- **Day 5: Documentation & Viva Preparation**
  - Finalize `README.md`, document architecture diagrams, and review viva question set.

---

## 8. Build Instructions & Root Makefile

### Root `Makefile`
```makefile
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude

LDFLAGS = -lpthread

SERVER_SRC = src/ResourceMonitor.cpp src/ProcessManager.cpp src/KernelMonitor.cpp src/Server.cpp src/main.cpp
SERVER_OBJ = $(SERVER_SRC:.cpp=.o)
SERVER_TARGET = sentinel_os

CLIENT_SRC = src/Client.cpp
CLIENT_OBJ = $(CLIENT_SRC:.cpp=.o)
CLIENT_TARGET = sentinel_client

all: $(SERVER_TARGET) $(CLIENT_TARGET)

$(SERVER_TARGET): $(SERVER_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(SERVER_OBJ) $(LDFLAGS)

$(CLIENT_TARGET): $(CLIENT_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(CLIENT_OBJ) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(SERVER_TARGET) $(CLIENT_TARGET)

.PHONY: all clean
```

### Execution Commands
```bash
# 1. Compile User-Space Application (Server Daemon and Client CLI)
make all

# 2. Build Character Device Driver
cd driver
make
gcc sentinel_test.c -o sentinel_test

# 3. Load Kernel Module & Test Character Device
sudo insmod sentinel_driver.ko
./sentinel_test
sudo rmmod sentinel_driver
cd ..

# 4. Run Monitoring Server Daemon
./sentinel_os

# 5. Run Client CLI (In second terminal)
./sentinel_client
```

---

## 9. Testing Plan

| Test ID | Test Category | Objective | Verification Command / Step | Expected Result |
| :--- | :--- | :--- | :--- | :--- |
| **TP-01** | Telemetry | Verify CPU, RAM, Disk parsing | Call `ResourceMonitor` methods in `main` | Displays non-zero utilization % |
| **TP-02** | Process | Verify PID discovery and state | Call `ProcessManager::get_all_processes()` | Returns list of PIDs with `'S'` or `'R'` state |
| **TP-03** | Kernel Info | Verify system version and uptime | Call `KernelMonitor::get_system_info()` | Displays Linux kernel release and uptime seconds |
| **TP-04** | Networking | Verify remote client commands | Run `./sentinel_client`, send `GET_CPU` | Server returns `CPU_USAGE: X.XX%` |
| **TP-05** | Driver | Verify `/dev/sentinel` file ops | Run `./sentinel_test` | Driver logs `read` and `write` success in `dmesg` |

---

## 10. Recommended Git Commit Plan (GitHub Portfolio Log)

```bash
# Commit 1: Initial Repository Structure & Docs
git commit -m "feat: initialize repository structure, architecture docs, and Makefile"

# Commit 2: Telemetry Monitoring Subsystems
git commit -m "feat(telemetry): implement ResourceMonitor, ProcessManager, and KernelMonitor modules"

# Commit 3: TCP Client-Server Architecture
git commit -m "feat(network): implement multithreaded Server daemon and CLI Client monitoring tool"

# Commit 4: Linux Character Device Driver
git commit -m "feat(driver): implement Linux Character Device Driver (/dev/sentinel) with open, read, write"

# Commit 5: Full System Integration & Documentation
git commit -m "docs: finalize README.md, build instructions, and end-to-end integration tests"
```
