#ifndef SENTINEL_IOCTL_H
#define SENTINEL_IOCTL_H

#if defined(__KERNEL__)
#include <linux/ioctl.h>
#include <linux/types.h>
#else
#include <sys/ioctl.h>
#include <stdint.h>
#endif

#define SENTINEL_IOC_MAGIC 's'

// Struct returning driver operational status over IOCTL
struct sentinel_driver_status {
    uint32_t driver_version;        // Driver version bitmask (0x010000 -> 1.0.0)
    uint32_t major_number;          // Assigned character device Major Number
    uint32_t minor_number;          // Assigned character device Minor Number
    uint32_t total_reads;           // Total read() calls processed
    uint32_t total_writes;          // Total write() calls processed
    uint32_t buffer_data_len;       // Current bytes stored in device buffer
};

// Struct returning kernel memory telemetry over IOCTL
struct sentinel_kern_mem {
    uint64_t total_kernel_ram_kb;   // Total system RAM in KB
    uint64_t free_kernel_ram_kb;    // Free system RAM in KB
    uint32_t page_size_bytes;       // Kernel Page Size (typically 4096 bytes)
};

// IOCTL Command definitions using Linux kernel _IOR, _IOW, _IO macros
#define SENTINEL_IOCTL_GET_STATUS    _IOR(SENTINEL_IOC_MAGIC, 1, struct sentinel_driver_status)
#define SENTINEL_IOCTL_GET_KERN_MEM  _IOR(SENTINEL_IOC_MAGIC, 2, struct sentinel_kern_mem)
#define SENTINEL_IOCTL_RESET_BUFFER  _IO(SENTINEL_IOC_MAGIC, 3)

#endif // SENTINEL_IOCTL_H
