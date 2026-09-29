#ifndef SENTINEL_COMMON_H
#define SENTINEL_COMMON_H

#include <string>
#include <iostream>

namespace Sentinel {
    constexpr const char* VERSION = "1.0.0";
    constexpr const char* PLATFORM_NAME = "SentinelOS";
    constexpr int DEFAULT_TCP_PORT = 9090;
    constexpr const char* DEFAULT_LOG_FILE = "logs/sentinel.log";
    constexpr const char* DEVICE_PATH = "/dev/sentinel";

    enum class Status {
        SUCCESS = 0,
        ERROR_INIT_FAILED,
        ERROR_FILE_NOT_FOUND,
        ERROR_PERMISSION_DENIED,
        ERROR_NETWORK_FAILURE,
        ERROR_DRIVER_FAILURE
    };
}

#endif // SENTINEL_COMMON_H
