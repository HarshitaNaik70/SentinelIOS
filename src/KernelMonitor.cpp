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
