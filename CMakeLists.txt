cmake_minimum_required(VERSION 3.25)
project(untitled5)

set(CMAKE_CXX_STANDARD 14)

add_executable(server server.cpp)
add_executable(client client.cpp)

target_link_libraries(server PRIVATE ws2_32)
target_link_libraries(client PRIVATE ws2_32)
