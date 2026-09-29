/*
 * SentinelOS Virtual Character Device Driver (/dev/sentinel)
 * Author: Harshita Naik (B.Tech CSE, SOA University)
 * Domain: Linux Kernel Module & System Programming
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/mm.h>

#include "sentinel_ioctl.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Harshita Naik");
MODULE_DESCRIPTION("SentinelOS Virtual Character Device Driver with IOCTL Telemetry");
MODULE_VERSION("1.0.0");

#define DEVICE_NAME "sentinel"
#define CLASS_NAME "sentinel_class"
#define RING_BUFFER_SIZE 65536 // 64 KB

static int major_number;
static struct class* sentinel_class = NULL;
static struct device* sentinel_device = NULL;
static struct cdev sentinel_cdev;

static char* ring_buffer;
static unsigned int ring_head = 0;
static unsigned int ring_tail = 0;
static unsigned int total_events = 0;
static spinlock_t ring_lock;

static int sentinel_open(struct inode *inodep, struct file *filep) {
    pr_info("sentinel_driver: Device opened by PID %d\n", current->pid);
    return 0;
}

static int sentinel_release(struct inode *inodep, struct file *filep) {
    pr_info("sentinel_driver: Device closed\n");
    return 0;
}

static ssize_t sentinel_read(struct file *filep, char __user *buffer, size_t len, loff_t *offset) {
    unsigned long flags;
    size_t bytes_to_copy;

    spin_lock_irqsave(&ring_lock, flags);
    if (ring_head == ring_tail) {
        spin_unlock_irqrestore(&ring_lock, flags);
        return 0; // Buffer empty
    }

    bytes_to_copy = min(len, (size_t)(RING_BUFFER_SIZE - ring_tail));
    if (copy_to_user(buffer, ring_buffer + ring_tail, bytes_to_copy)) {
        spin_unlock_irqrestore(&ring_lock, flags);
        return -EFAULT;
    }

    ring_tail = (ring_tail + bytes_to_copy) % RING_BUFFER_SIZE;
    spin_unlock_irqrestore(&ring_lock, flags);

    return bytes_to_copy;
}

static long sentinel_ioctl(struct file *filep, unsigned int cmd, unsigned long arg) {
    struct sentinel_driver_status status;
    struct sentinel_kern_mem kmem;

    switch (cmd) {
        case SENTINEL_IOCTL_GET_STATUS:
            status.driver_version = 0x010000;
            status.ring_buffer_head = ring_head;
            status.ring_buffer_tail = ring_tail;
            status.total_events_logged = total_events;
            if (copy_to_user((void __user *)arg, &status, sizeof(status))) {
                return -EFAULT;
            }
            break;

        case SENTINEL_IOCTL_GET_KERN_MEM:
            kmem.total_kernel_ram = totalram_pages() * (PAGE_SIZE / 1024);
            kmem.free_kernel_ram = nr_free_pages() * (PAGE_SIZE / 1024);
            kmem.total_high_mem = 0;
            kmem.free_high_mem = 0;
            kmem.active_module_count = 12;
            if (copy_to_user((void __user *)arg, &kmem, sizeof(kmem))) {
                return -EFAULT;
            }
            break;

        case SENTINEL_IOCTL_RESET_RINGBUF:
            spin_lock(&ring_lock);
            ring_head = 0;
            ring_tail = 0;
            spin_unlock(&ring_lock);
            pr_info("sentinel_driver: Ring buffer reset via IOCTL\n");
            break;

        default:
            return -EINVAL;
    }
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = sentinel_open,
    .read = sentinel_read,
    .release = sentinel_release,
    .unlocked_ioctl = sentinel_ioctl,
};

static int __init sentinel_init(void) {
    dev_t dev;
    pr_info("sentinel_driver: Initializing SentinelOS Driver...\n");

    if (alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME) < 0) {
        pr_err("sentinel_driver: Failed to allocate major number\n");
        return -1;
    }
    major_number = MAJOR(dev);

    cdev_init(&sentinel_cdev, &fops);
    if (cdev_add(&sentinel_cdev, dev, 1) < 0) {
        unregister_chrdev_region(dev, 1);
        return -1;
    }

    sentinel_class = class_create(CLASS_NAME);
    if (IS_ERR(sentinel_class)) {
        cdev_del(&sentinel_cdev);
        unregister_chrdev_region(dev, 1);
        return PTR_ERR(sentinel_class);
    }

    sentinel_device = device_create(sentinel_class, NULL, dev, NULL, DEVICE_NAME);
    if (IS_ERR(sentinel_device)) {
        class_destroy(sentinel_class);
        cdev_del(&sentinel_cdev);
        unregister_chrdev_region(dev, 1);
        return PTR_ERR(sentinel_device);
    }

    ring_buffer = kmalloc(RING_BUFFER_SIZE, GFP_KERNEL);
    if (!ring_buffer) {
        device_destroy(sentinel_class, dev);
        class_destroy(sentinel_class);
        cdev_del(&sentinel_cdev);
        unregister_chrdev_region(dev, 1);
        return -ENOMEM;
    }

    spin_lock_init(&ring_lock);
    pr_info("sentinel_driver: Registered /dev/%s with major %d\n", DEVICE_NAME, major_number);
    return 0;
}

static void __exit sentinel_exit(void) {
    dev_t dev = MKDEV(major_number, 0);
    kfree(ring_buffer);
    device_destroy(sentinel_class, dev);
    class_destroy(sentinel_class);
    cdev_del(&sentinel_cdev);
    unregister_chrdev_region(dev, 1);
    pr_info("sentinel_driver: Driver unloaded successfully\n");
}

module_init(sentinel_init);
module_exit(sentinel_exit);
