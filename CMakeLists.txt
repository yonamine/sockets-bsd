cmake_minimum_required(VERSION 3.15)
project(SOCKETS_BSD)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_BUILD_TYPE "Debug")

find_package(Threads REQUIRED)

add_executable(socket_client "socket_client.cpp")
target_link_libraries(socket_client Threads::Threads)

add_executable(socket_server "socket_server.cpp")
target_link_libraries(socket_server Threads::Threads)
