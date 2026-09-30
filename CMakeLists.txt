cmake_minimum_required(VERSION 3.20)
project(SentinelOS VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include client driver)

find_package(Threads REQUIRED)

# 1. Server Daemon Executable Target
add_executable(sentinel_os
    src/Server.cpp
    src/ProcessManager.cpp
    src/KernelMonitor.cpp
    src/ResourceMonitor.cpp
    src/main.cpp
)

target_link_libraries(sentinel_os PRIVATE Threads::Threads)
if(WIN32)
    target_link_libraries(sentinel_os PRIVATE ws2_32)
endif()

# 2. Client Application Executable Target
add_executable(sentinel_client
    client/Client.cpp
    client/client_main.cpp
)

target_link_libraries(sentinel_client PRIVATE Threads::Threads)
if(WIN32)
    target_link_libraries(sentinel_client PRIVATE ws2_32)
endif()
