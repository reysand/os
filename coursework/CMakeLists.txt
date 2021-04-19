cmake_minimum_required(VERSION 3.19)
project(coursework)

set(CMAKE_CXX_STANDARD 20)

LINK_LIBRARIES(ws2_32.lib)

add_executable(client src/client.cpp)
add_executable(server1 src/server1.cpp)
add_executable(server2 src/server2.cpp)
