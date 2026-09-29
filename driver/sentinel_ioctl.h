#ifndef SENTINEL_IOCTL_H
#define SENTINEL_IOCTL_H

#if defined(__KERNEL__)
#include <linux/ioctl.h>
#else
#include <sys/ioctl.h>
#include <cstdint>
#endif

#define SENTINEL_IOC_MAGIC 's'

struct sentinel_driver_status {
    unsigned int driver_version;
    unsigned int ring_buffer_head;
    unsigned int ring_buffer_tail;
    unsigned int total_events_logged;
};

struct sentinel_kern_mem {
    unsigned long total_kernel_ram;
    unsigned long free_kernel_ram;
    unsigned long total_high_mem;
    unsigned long free_high_mem;
    unsigned int active_module_count;
};

#define SENTINEL_IOCTL_GET_STATUS    _IOR(SENTINEL_IOC_MAGIC, 1, struct sentinel_driver_status)
#define SENTINEL_IOCTL_GET_KERN_MEM  _IOR(SENTINEL_IOC_MAGIC, 2, struct sentinel_kern_mem)
#define SENTINEL_IOCTL_RESET_RINGBUF _IO(SENTINEL_IOC_MAGIC, 3)
#define SENTINEL_IOCTL_SET_LOG_LEVEL _IOW(SENTINEL_IOC_MAGIC, 4, int)

#endif // SENTINEL_IOCTL_H
