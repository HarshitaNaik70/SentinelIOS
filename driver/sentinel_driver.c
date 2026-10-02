/*
 * SentinelOS Autonomous Health Monitoring Platform
 * Module: Linux Character Device Driver (/dev/sentinel)
 *
 * Description:
 * Linux Kernel Module providing a character device interface (/dev/sentinel)
 * for secure user-space <-> kernel-space telemetry exchange, state control,
 * and system telemetry reads using POSIX file operations (open, read, write, ioctl, release).
 *
 * Author: Senior Linux Kernel Developer & Device Driver Engineer
 * License: GPL v2
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/version.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/swap.h>

#include "sentinel_ioctl.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SentinelOS Core Kernel Team");
MODULE_DESCRIPTION("SentinelOS Linux Character Device Driver for User-Kernel Telemetry");
MODULE_VERSION("1.0.0");

#define DEVICE_NAME "sentinel"
#define CLASS_NAME "sentinel_class"
#define DEVICE_BUFFER_SIZE 4096

// Driver Global State Variables
static int major_number = 0;
static struct class* sentinel_class = NULL;
static struct device* sentinel_device = NULL;
static struct cdev sentinel_cdev;

static char device_buffer[DEVICE_BUFFER_SIZE];
static size_t buffer_data_len = 0;
static uint32_t total_reads_count = 0;
static uint32_t total_writes_count = 0;

static DEFINE_MUTEX(sentinel_mutex);

/**
 * @brief Handles device open() system calls from user-space.
 */
static int sentinel_open(struct inode *inodep, struct file *filep) {
    pr_info("sentinel_driver: /dev/%s opened by PID %d (comm: %s)\n", 
            DEVICE_NAME, current->pid, current->comm);
    return 0;
}

/**
 * @brief Handles device release() / close() system calls from user-space.
 */
static int sentinel_release(struct inode *inodep, struct file *filep) {
    pr_info("sentinel_driver: /dev/%s closed by PID %d\n", DEVICE_NAME, current->pid);
    return 0;
}

/**
 * @brief Handles read() system calls from user-space.
 * Safely transfers data from kernel-space device_buffer to user-space buffer via copy_to_user().
 */
static ssize_t sentinel_read(struct file *filep, char __user *buffer, size_t len, loff_t *offset) {
    size_t bytes_to_read = 0;
    unsigned long uncopied_bytes = 0;

    if (mutex_lock_interruptible(&sentinel_mutex)) {
        return -ERESTARTSYS;
    }

    // Check End-Of-File (EOF) condition
    if (*offset >= buffer_data_len) {
        mutex_unlock(&sentinel_mutex);
        return 0;
    }

    bytes_to_read = min(len, (size_t)(buffer_data_len - *offset));
    uncopied_bytes = copy_to_user(buffer, device_buffer + *offset, bytes_to_read);

    if (uncopied_bytes == 0) {
        *offset += bytes_to_read;
        total_reads_count++;
        pr_info("sentinel_driver: Sent %zu bytes to user-space PID %d\n", bytes_to_read, current->pid);
        mutex_unlock(&sentinel_mutex);
        return bytes_to_read;
    } else {
        size_t bytes_copied = bytes_to_read - uncopied_bytes;
        *offset += bytes_copied;
        pr_err("sentinel_driver: Partial copy_to_user failure (%lu bytes uncopied)\n", uncopied_bytes);
        mutex_unlock(&sentinel_mutex);
        return bytes_copied > 0 ? bytes_copied : -EFAULT;
    }
}

/**
 * @brief Handles write() system calls from user-space.
 * Safely receives command/telemetry messages from user-space via copy_from_user().
 */
static ssize_t sentinel_write(struct file *filep, const char __user *buffer, size_t len, loff_t *offset) {
    size_t bytes_to_copy = 0;
    unsigned long uncopied_bytes = 0;

    if (mutex_lock_interruptible(&sentinel_mutex)) {
        return -ERESTARTSYS;
    }

    bytes_to_copy = min(len, (size_t)(DEVICE_BUFFER_SIZE - 1));
    memset(device_buffer, 0, DEVICE_BUFFER_SIZE);

    uncopied_bytes = copy_from_user(device_buffer, buffer, bytes_to_copy);
    if (uncopied_bytes == 0) {
        device_buffer[bytes_to_copy] = '\0';
        buffer_data_len = bytes_to_copy;
        total_writes_count++;

        pr_info("sentinel_driver: Received %zu bytes from user PID %d: '%s'\n", 
                bytes_to_copy, current->pid, device_buffer);
        mutex_unlock(&sentinel_mutex);
        return bytes_to_copy;
    } else {
        pr_err("sentinel_driver: Failed copy_from_user (%lu bytes uncopied)\n", uncopied_bytes);
        mutex_unlock(&sentinel_mutex);
        return -EFAULT;
    }
}

/**
 * @brief Handles ioctl() system calls for structured driver control and kernel memory queries.
 */
static long sentinel_ioctl(struct file *filep, unsigned int cmd, unsigned long arg) {
    struct sentinel_driver_status status;
    struct sentinel_kern_mem kmem;

    switch (cmd) {
        case SENTINEL_IOCTL_GET_STATUS:
            status.driver_version = 0x010000; // Version 1.0.0
            status.major_number = major_number;
            status.minor_number = 0;
            status.total_reads = total_reads_count;
            status.total_writes = total_writes_count;
            status.buffer_data_len = (uint32_t)buffer_data_len;

            if (copy_to_user((void __user *)arg, &status, sizeof(status))) {
                pr_err("sentinel_driver: IOCTL GET_STATUS copy_to_user failed\n");
                return -EFAULT;
            }
            pr_info("sentinel_driver: Executed IOCTL GET_STATUS for PID %d\n", current->pid);
            break;

        case SENTINEL_IOCTL_GET_KERN_MEM:
            kmem.total_kernel_ram_kb = (uint64_t)totalram_pages() * (PAGE_SIZE / 1024);
            kmem.free_kernel_ram_kb = (uint64_t)nr_free_pages() * (PAGE_SIZE / 1024);
            kmem.page_size_bytes = PAGE_SIZE;

            if (copy_to_user((void __user *)arg, &kmem, sizeof(kmem))) {
                pr_err("sentinel_driver: IOCTL GET_KERN_MEM copy_to_user failed\n");
                return -EFAULT;
            }
            pr_info("sentinel_driver: Executed IOCTL GET_KERN_MEM for PID %d\n", current->pid);
            break;

        case SENTINEL_IOCTL_RESET_BUFFER:
            mutex_lock(&sentinel_mutex);
            memset(device_buffer, 0, DEVICE_BUFFER_SIZE);
            snprintf(device_buffer, DEVICE_BUFFER_SIZE, "[SentinelOS Kernel Engine Ready - Buffer Reset]\n");
            buffer_data_len = strlen(device_buffer);
            mutex_unlock(&sentinel_mutex);

            pr_info("sentinel_driver: Reset device buffer via IOCTL by PID %d\n", current->pid);
            break;

        default:
            pr_warn("sentinel_driver: Invalid IOCTL command 0x%X from PID %d\n", cmd, current->pid);
            return -EINVAL;
    }

    return 0;
}

// File Operations Table binding VFS interface to character driver implementations
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = sentinel_open,
    .release = sentinel_release,
    .read = sentinel_read,
    .write = sentinel_write,
    .unlocked_ioctl = sentinel_ioctl,
};

/**
 * @brief Kernel Module Initialization entry point.
 */
static int __init sentinel_init(void) {
    dev_t dev;
    int ret = 0;

    pr_info("sentinel_driver: Initializing SentinelOS Linux Character Device Driver...\n");

    // 1. Dynamically allocate Major & Minor Numbers
    ret = alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        pr_err("sentinel_driver: Failed to allocate chrdev region (Error: %d)\n", ret);
        return ret;
    }
    major_number = MAJOR(dev);

    // 2. Initialize and Register Character Device (cdev) with VFS
    cdev_init(&sentinel_cdev, &fops);
    sentinel_cdev.owner = THIS_MODULE;

    ret = cdev_add(&sentinel_cdev, dev, 1);
    if (ret < 0) {
        pr_err("sentinel_driver: Failed to add cdev to system (Error: %d)\n", ret);
        unregister_chrdev_region(dev, 1);
        return ret;
    }

    // 3. Create sysfs device class for auto-creation of /dev node
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
    sentinel_class = class_create(CLASS_NAME);
#else
    sentinel_class = class_create(THIS_MODULE, CLASS_NAME);
#endif

    if (IS_ERR(sentinel_class)) {
        pr_err("sentinel_driver: Failed to create device class\n");
        cdev_del(&sentinel_cdev);
        unregister_chrdev_region(dev, 1);
        return PTR_ERR(sentinel_class);
    }

    // 4. Create device node /dev/sentinel
    sentinel_device = device_create(sentinel_class, NULL, dev, NULL, DEVICE_NAME);
    if (IS_ERR(sentinel_device)) {
        pr_err("sentinel_driver: Failed to create device node /dev/%s\n", DEVICE_NAME);
        class_destroy(sentinel_class);
        cdev_del(&sentinel_cdev);
        unregister_chrdev_region(dev, 1);
        return PTR_ERR(sentinel_device);
    }

    // Initialize default welcome message in kernel device buffer
    snprintf(device_buffer, DEVICE_BUFFER_SIZE, "[SentinelOS Kernel Subsystem Active - Major: %d, Minor: 0]\n", major_number);
    buffer_data_len = strlen(device_buffer);

    pr_info("sentinel_driver: Successfully registered character device /dev/%s (Major: %d, Minor: 0)\n", 
            DEVICE_NAME, major_number);
    return 0;
}

/**
 * @brief Kernel Module Cleanup entry point.
 */
static void __exit sentinel_exit(void) {
    dev_t dev = MKDEV(major_number, 0);

    device_destroy(sentinel_class, dev);
    class_destroy(sentinel_class);
    cdev_del(&sentinel_cdev);
    unregister_chrdev_region(dev, 1);

    pr_info("sentinel_driver: Unloaded /dev/%s driver and cleaned up kernel resources successfully.\n", DEVICE_NAME);
}

module_init(sentinel_init);
module_exit(sentinel_exit);
