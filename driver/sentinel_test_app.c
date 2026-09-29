/*
 * SentinelOS User-Space Character Device Communication Test Application
 *
 * Description:
 * User-space C test program demonstrating open(), read(), write(), and ioctl()
 * interaction with the SentinelOS Linux Character Device Driver (/dev/sentinel).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "sentinel_ioctl.h"

#define DEVICE_PATH "/dev/sentinel"
#define READ_BUF_SIZE 1024

int main() {
    int fd = -1;
    ssize_t bytes_read = 0;
    ssize_t bytes_written = 0;
    char read_buffer[READ_BUF_SIZE] = {0};
    const char *write_message = "SENTINEL_CMD: ENABLE_PROCESS_WATCHDOG_MODE";
    
    struct sentinel_driver_status status;
    struct sentinel_kern_mem kmem;

    printf("=================================================================\n");
    printf("   SentinelOS User-Space Kernel Device Communication Test        \n");
    printf("=================================================================\n");

    // 1. Open Character Device /dev/sentinel
    printf("[1] Opening character device %s...\n", DEVICE_PATH);
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("[ERROR] Failed to open /dev/sentinel");
        printf("[HINT] Ensure kernel module is loaded: 'sudo insmod sentinel_driver.ko'\n");
        printf("[HINT] Ensure permissions are set: 'sudo chmod 666 /dev/sentinel'\n");
        return 1;
    }
    printf("[SUCCESS] Opened %s successfully (File Descriptor: %d)\n\n", DEVICE_PATH, fd);

    // 2. Read initial data from driver
    printf("[2] Reading telemetry banner from kernel device...\n");
    bytes_read = read(fd, read_buffer, sizeof(read_buffer) - 1);
    if (bytes_read < 0) {
        perror("[ERROR] Read failed");
    } else {
        read_buffer[bytes_read] = '\0';
        printf("[DRIVER READ RESULT] Received %zd bytes:\n%s\n", bytes_read, read_buffer);
    }

    // 3. Write command message into kernel driver
    printf("[3] Sending command message to kernel device...\n");
    printf("    Write Payload: '%s'\n", write_message);
    bytes_written = write(fd, write_message, strlen(write_message));
    if (bytes_written < 0) {
        perror("[ERROR] Write failed");
    } else {
        printf("[SUCCESS] Wrote %zd bytes to kernel device.\n\n", bytes_written);
    }

    // 4. Query Driver Telemetry Status over IOCTL
    printf("[4] Querying Driver Telemetry Status over IOCTL (SENTINEL_IOCTL_GET_STATUS)...\n");
    if (ioctl(fd, SENTINEL_IOCTL_GET_STATUS, &status) < 0) {
        perror("[ERROR] IOCTL GET_STATUS failed");
    } else {
        printf("    [IOCTL Telemetry Result]\n");
        printf("    - Driver Version : %u.%u.%u\n", 
               (status.driver_version >> 16) & 0xFF,
               (status.driver_version >> 8) & 0xFF,
               status.driver_version & 0xFF);
        printf("    - Major Number   : %u\n", status.major_number);
        printf("    - Minor Number   : %u\n", status.minor_number);
        printf("    - Total Reads    : %u\n", status.total_reads);
        printf("    - Total Writes   : %u\n", status.total_writes);
        printf("    - Buffer Data Len: %u bytes\n\n", status.buffer_data_len);
    }

    // 5. Query Kernel Memory Information over IOCTL
    printf("[5] Querying Kernel Memory Telemetry over IOCTL (SENTINEL_IOCTL_GET_KERN_MEM)...\n");
    if (ioctl(fd, SENTINEL_IOCTL_GET_KERN_MEM, &kmem) < 0) {
        perror("[ERROR] IOCTL GET_KERN_MEM failed");
    } else {
        printf("    [Kernel Memory Telemetry]\n");
        printf("    - Total Kernel RAM : %lu MB (%lu KB)\n", kmem.total_kernel_ram_kb / 1024, kmem.total_kernel_ram_kb);
        printf("    - Free Kernel RAM  : %lu MB (%lu KB)\n", kmem.free_kernel_ram_kb / 1024, kmem.free_kernel_ram_kb);
        printf("    - Page Size        : %u bytes\n\n", kmem.page_size_bytes);
    }

    // 6. Close Character Device handle
    printf("[6] Closing device handle...\n");
    close(fd);
    printf("[SUCCESS] Closed /dev/sentinel. Communication test completed successfully.\n");
    printf("=================================================================\n");

    return 0;
}
