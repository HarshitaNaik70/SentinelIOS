#include "KernelMonitor.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cstdint>
#if defined(__linux__) || defined(__gnu_linux__) || defined(__unix__)
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <unistd.h>
#endif

// Helper Function: Safely read the first line of a file
std::string KernelMonitor::read_first_line(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return "Unavailable";
    }
    std::string line;
    if (std::getline(file, line)) {
        return line;
    }
    return "Unavailable";
}

// 1. Reads Kernel Version Information from /proc/version and uname()
KernelVersionInfo KernelMonitor::get_kernel_version_info() {
    KernelVersionInfo info;
    info.full_kernel_version = read_first_line("/proc/version");
    info.kernel_release = "Linux 6.x";
    info.os_distro = "Ubuntu Linux";

#if defined(__linux__)
    struct utsname buf;
    if (uname(&buf) == 0) {
        info.kernel_release = std::string(buf.sysname) + " " + std::string(buf.release);
        info.os_distro = std::string(buf.sysname) + " (" + std::string(buf.machine) + ")";
    }
#endif

    return info;
}

// 2. Reads CPU Information from /proc/cpuinfo
CpuHardwareInfo KernelMonitor::get_cpu_info() {
    CpuHardwareInfo cpu;
    cpu.model_name = "Generic x86_64 Processor";
    cpu.num_cores = 1;
    cpu.architecture = "x86_64";

#if defined(__linux__)
    struct utsname buf;
    if (uname(&buf) == 0) {
        cpu.architecture = buf.machine;
    }

    std::ifstream file("/proc/cpuinfo");
    if (file.is_open()) {
        std::string line;
        int core_count = 0;
        bool model_found = false;

        while (std::getline(file, line)) {
            // Count processor cores
            if (line.rfind("processor", 0) == 0) {
                core_count++;
            }

            // Parse model name
            if (!model_found && (line.find("model name") != std::string::npos || line.find("Model") != std::string::npos)) {
                size_t colon_pos = line.find(':');
                if (colon_pos != std::string::npos) {
                    cpu.model_name = line.substr(colon_pos + 2);
                    model_found = true;
                }
            }
        }

        if (core_count > 0) {
            cpu.num_cores = core_count;
        }
    }
#endif

    return cpu;
}

// 3. Reads System Information (Hostname, Uptime, OS Details)
SystemInfo KernelMonitor::get_system_info() {
    SystemInfo sys;
    sys.hostname = "localhost";
    sys.uptime_seconds = 0.0;
    sys.os_details = "GNU/Linux";

#if defined(__linux__)
    char hostname_buf[256];
    if (gethostname(hostname_buf, sizeof(hostname_buf)) == 0) {
        sys.hostname = std::string(hostname_buf);
    }

    struct sysinfo s_info;
    if (sysinfo(&s_info) == 0) {
        sys.uptime_seconds = static_cast<double>(s_info.uptime);
    }
#else
    sys.uptime_seconds = 3600.0;
#endif

    return sys;
}

// 4. Reads Memory and Swap Information from /proc/meminfo
MemorySwapInfo KernelMonitor::get_memory_swap_info() {
    MemorySwapInfo mem{};
    std::ifstream file("/proc/meminfo");
    if (!file.is_open()) {
        return mem;
    }

    std::string key;
    uint64_t value;
    std::string unit;

    uint64_t total_ram_kb = 0;
    uint64_t avail_ram_kb = 0;
    uint64_t total_swap_kb = 0;
    uint64_t free_swap_kb = 0;

    while (file >> key >> value >> unit) {
        if (key == "MemTotal:") {
            total_ram_kb = value;
        } else if (key == "MemAvailable:") {
            avail_ram_kb = value;
        } else if (key == "SwapTotal:") {
            total_swap_kb = value;
        } else if (key == "SwapFree:") {
            free_swap_kb = value;
        }
    }

    mem.total_ram_mb = static_cast<double>(total_ram_kb) / 1024.0;
    mem.available_ram_mb = static_cast<double>(avail_ram_kb) / 1024.0;
    mem.total_swap_mb = static_cast<double>(total_swap_kb) / 1024.0;
    mem.free_swap_mb = static_cast<double>(free_swap_kb) / 1024.0;

    std::cout << "DEBUG Total RAM KB = " << total_ram_kb << std::endl;
std::cout << "DEBUG Available RAM KB = " << avail_ram_kb << std::endl;
std::cout << "DEBUG Total Swap KB = " << total_swap_kb << std::endl;
std::cout << "DEBUG Free Swap KB = " << free_swap_kb << std::endl;

    return mem;
}

// 5. Displays formatted summary of Kernel Information
void KernelMonitor::display_kernel_summary() {
    KernelVersionInfo kver = get_kernel_version_info();
    CpuHardwareInfo cpu = get_cpu_info();
    SystemInfo sys = get_system_info();
    MemorySwapInfo mem = get_memory_swap_info();

    std::cout << "\n----------------------------------------------------------" << std::endl;
    std::cout << "               LINUX KERNEL & SYSTEM SUMMARY              " << std::endl;
    std::cout << "----------------------------------------------------------" << std::endl;
    std::cout << " Hostname      : " << sys.hostname << std::endl;
    std::cout << " Kernel Release: " << kver.kernel_release << std::endl;
    std::cout << " OS Distro     : " << kver.os_distro << std::endl;
    std::cout << " CPU Model     : " << cpu.model_name << std::endl;
    std::cout << " CPU Cores     : " << cpu.num_cores << " Core(s) (" << cpu.architecture << ")" << std::endl;
    std::cout << " System Uptime : " << std::fixed << std::setprecision(1) 
              << (sys.uptime_seconds / 3600.0) << " hours (" << sys.uptime_seconds << " seconds)" << std::endl;
    std::cout << " Total RAM     : " << std::setprecision(1) << mem.total_ram_mb << " MB (Available: " << mem.available_ram_mb << " MB)" << std::endl;
    std::cout << " Total Swap    : " << mem.total_swap_mb << " MB (Free: " << mem.free_swap_mb << " MB)" << std::endl;
    std::cout << "----------------------------------------------------------" << std::endl;
}
