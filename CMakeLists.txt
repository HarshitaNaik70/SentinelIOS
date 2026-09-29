cmake_minimum_required(VERSION 3.20)
project(SentinelOS VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include client)

# 1. Server Daemon Target
file(GLOB SERVER_SOURCES "src/*.cpp")
add_executable(sentinel_os ${SERVER_SOURCES})

# 2. Client Application Target
add_executable(sentinel_client client/Client.cpp client/client_main.cpp src/Logger.cpp)
