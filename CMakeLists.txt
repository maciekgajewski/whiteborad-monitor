project(whiteboard-monitor)
cmake_minimum_required(VERSION 3.1.0)

#include_directories(external/fmt/include)

add_subdirectory(external)

add_compile_options(-Wall)
add_subdirectory(monitor)
