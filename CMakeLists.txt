cmake_minimum_required(VERSION 3.18)
project(p_code)

set(CMAKE_CXX_STANDARD 20)

add_executable(p_code main.cpp stack.h stack.cpp
        mem.cpp
        mem.h
)
