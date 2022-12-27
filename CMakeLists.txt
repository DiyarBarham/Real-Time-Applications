cmake_minimum_required(VERSION 3.21)
project(Real_Time C)

set(CMAKE_C_STANDARD 11)

add_executable(Real_Time
        "Project 1/child.c"
        "Project 1/parent.c"
        "Project 2/parent.c")
