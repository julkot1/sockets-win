cmake_minimum_required(VERSION 3.26)
project(server C)

set(CMAKE_C_STANDARD 11)

add_executable(server main.c
        socket.c
        socket.h
        gui.c
        gui.h)
target_link_libraries(server PRIVATE ws2_32)
