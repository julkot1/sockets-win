cmake_minimum_required(VERSION 3.26)
project(client C)

set(CMAKE_C_STANDARD 11)

add_executable(client main.c)
target_link_libraries(client PRIVATE ws2_32)
